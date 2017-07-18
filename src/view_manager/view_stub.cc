// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/view_manager/view_stub.h"

#include <utility>

#include "apps/mozart/src/view_manager/view_registry.h"
#include "apps/mozart/src/view_manager/view_state.h"
#include "apps/mozart/src/view_manager/view_tree_state.h"
#include "lib/ftl/logging.h"

namespace view_manager {

class PendingViewOwnerTransferState {
 public:
  PendingViewOwnerTransferState(
      std::unique_ptr<ViewStub> view_stub,
      fidl::InterfaceRequest<mozart::ViewOwner> transferred_view_owner_request)
      : view_stub_(std::move(view_stub)),
        transferred_view_owner_request_(
            std::move(transferred_view_owner_request)) {}

  ~PendingViewOwnerTransferState() {}

  // A reference to keep the |ViewStub| alive until |OnViewResolved| is called.
  std::unique_ptr<ViewStub> view_stub_;

  // The |ViewOwner| we want to transfer ownership to.
  fidl::InterfaceRequest<mozart::ViewOwner> transferred_view_owner_request_;
};

ViewStub::ViewStub(ViewRegistry* registry,
                   fidl::InterfaceHandle<mozart::ViewOwner> owner,
                   mx::eventpair host_import_token)
    : registry_(registry),
      owner_(mozart::ViewOwnerPtr::Create(std::move(owner))),
      host_import_token_(std::move(host_import_token)),
      weak_factory_(this) {
  FTL_DCHECK(registry_);
  FTL_DCHECK(owner_);
  FTL_DCHECK(host_import_token_);

  owner_.set_connection_error_handler([this] { OnViewResolved(nullptr); });

  owner_->GetToken([this](mozart::ViewTokenPtr view_token) {
    OnViewResolved(std::move(view_token));
  });
}

ViewStub::~ViewStub() {
  // Ensure that everything was properly released before this object was
  // destroyed.  The |ViewRegistry| is responsible for maintaining the
  // invariant that all |ViewState| objects are owned so by the time we
  // get here, the view should have found a new owner or been unregistered.
  FTL_DCHECK(is_unavailable());
}

ViewContainerState* ViewStub::container() const {
  return parent_ ? static_cast<ViewContainerState*>(parent_) : tree_;
}

void ViewStub::AttachView(ViewState* state) {
  FTL_DCHECK(state);
  FTL_DCHECK(!state->view_stub());
  FTL_DCHECK(is_pending());

  state_ = state;
  state_->set_view_stub(this);
  SetTreeForChildrenOfView(state_, tree_);
}

void ViewStub::SetProperties(mozart::ViewPropertiesPtr properties) {
  FTL_DCHECK(!is_unavailable());

  properties_ = std::move(properties);
}

ViewState* ViewStub::ReleaseView() {
  if (is_unavailable())
    return nullptr;

  ViewState* state = state_;
  if (state) {
    FTL_DCHECK(state->view_stub() == this);
    state->set_view_stub(nullptr);
    state_ = nullptr;
    SetTreeForChildrenOfView(state, nullptr);
  }
  properties_.reset();
  unavailable_ = true;
  return state;
}

void ViewStub::SetContainer(ViewContainerState* container, uint32_t key) {
  FTL_DCHECK(container);
  FTL_DCHECK(!tree_ && !parent_);

  key_ = key;
  parent_ = container->AsViewState();
  if (parent_) {
    if (parent_->view_stub())
      SetTreeRecursively(parent_->view_stub()->tree());
  } else {
    ViewTreeState* tree = container->AsViewTreeState();
    FTL_DCHECK(tree);
    SetTreeRecursively(tree);
  }
}

void ViewStub::Unlink() {
  parent_ = nullptr;
  key_ = 0;
  SetTreeRecursively(nullptr);
}

void ViewStub::SetTreeRecursively(ViewTreeState* tree) {
  if (tree_ == tree)
    return;
  tree_ = tree;
  if (state_)
    SetTreeForChildrenOfView(state_, tree);
}

void ViewStub::SetTreeForChildrenOfView(ViewState* view, ViewTreeState* tree) {
  for (const auto& pair : view->children()) {
    pair.second->SetTreeRecursively(tree);
  }
}

// Called when the ViewOwner returns a token (using GetToken), or when the
// ViewOwner is disconnected.
void ViewStub::OnViewResolved(mozart::ViewTokenPtr view_token) {
  if (view_token && transfer_view_owner_when_view_resolved()) {
    // While we were waiting for GetToken(), the view was transferred to a new
    // ViewOwner). Now that we got the GetToken() call, transfer the ownership
    // correctly internally.
    FTL_DCHECK(!container());  // Make sure we're removed from the view tree
    FTL_DCHECK(pending_view_owner_transfer_->view_stub_ != nullptr);
    FTL_DCHECK(pending_view_owner_transfer_->transferred_view_owner_request_
                   .is_pending());
    FTL_DCHECK(owner_);
    owner_.reset();

    registry_->TransferViewOwner(
        std::move(view_token),
        std::move(
            pending_view_owner_transfer_->transferred_view_owner_request_));

    // We don't have any |view_state| resolved to us now, but |ReleaseView| will
    // still mark us as unavailable and clear properties
    ReleaseView();

    // |pending_view_owner_transfer_| holds a reference to ourselves. Don't hold
    // that reference anymore, which should release us immediately.
    pending_view_owner_transfer_.reset();
  } else {
    // 1. We got the ViewOwner GetToken() callback as expected.
    // 2. Or, the ViewOwner was closed before the GetToken() callback (in
    // which case view_token is null).
    FTL_DCHECK(owner_);
    owner_.reset();
    registry_->OnViewResolved(this, std::move(view_token));
  }
}

void ViewStub::TransferViewOwnerWhenViewResolved(
    std::unique_ptr<ViewStub> view_stub,
    fidl::InterfaceRequest<mozart::ViewOwner> transferred_view_owner_request) {
  FTL_DCHECK(!container());  // Make sure we've been removed from the view tree
  FTL_DCHECK(!pending_view_owner_transfer_);

  // When |OnViewResolved| gets called, we'll just transfer ownership
  // of the view instead of calling |ViewRegistry.OnViewResolved|.
  // Save the necessary state in |pending_view_owner_transfer_|
  pending_view_owner_transfer_.reset(new PendingViewOwnerTransferState(
      std::move(view_stub), std::move(transferred_view_owner_request)));

  // TODO(mikejurka): should we have an error handler on
  // transferred_view_owner_request_?
}

void ViewStub::ReleaseHost() {
  host_import_token_.reset();
  host_node_.reset();
}

void ViewStub::ImportHostNode(mozart::client::Session* session) {
  FTL_DCHECK(host_import_token_);
  FTL_DCHECK(!host_node_);

  host_node_.reset(new mozart::client::ImportNode(session));
  host_node_->Bind(std::move(host_import_token_));
}

}  // namespace view_manager
