// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_VIEW_MANAGER_VIEW_CONTAINER_STATE_H_
#define APPS_MOZART_SRC_VIEW_MANAGER_VIEW_CONTAINER_STATE_H_

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>

#include "apps/mozart/services/views/view_containers.fidl.h"
#include "apps/mozart/src/view_manager/view_stub.h"
#include "lib/ftl/macros.h"

namespace view_manager {

class ViewState;
class ViewStub;
class ViewTreeState;

// Base class for views and view trees.
// This object is owned by the ViewRegistry that created it.
class ViewContainerState {
 public:
  using ChildrenMap = std::unordered_map<uint32_t, std::unique_ptr<ViewStub>>;

  ViewContainerState();

  // Gets or sets the view container listener.
  mozart::ViewContainerListener* view_container_listener() const {
    return view_container_listener_.get();
  }
  void set_view_container_listener(
      mozart::ViewContainerListenerPtr view_container_listener) {
    view_container_listener_ = std::move(view_container_listener);
  }

  // The map of children, indexed by child key.
  // The view stub pointers are never null but some view stubs may
  // have been marked unavailable.
  const ChildrenMap& children() const { return children_; }

  // Links a child into the view tree.
  void LinkChild(uint32_t key, std::unique_ptr<ViewStub> child);

  // Unlinks a child of the view tree.
  std::unique_ptr<ViewStub> UnlinkChild(uint32_t key);

  // Unlinks all children as a single operation.
  std::vector<std::unique_ptr<ViewStub>> UnlinkAllChildren();

  // Dynamic type tests (ugh).
  virtual ViewState* AsViewState();
  virtual ViewTreeState* AsViewTreeState();

  virtual const std::string& FormattedLabel() const = 0;

 protected:
  virtual ~ViewContainerState();

 private:
  mozart::ViewContainerListenerPtr view_container_listener_;
  ChildrenMap children_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ViewContainerState);
};

std::ostream& operator<<(std::ostream& os, ViewContainerState* view_state);

}  // namespace view_manager

#endif  // APPS_MOZART_SRC_VIEW_MANAGER_VIEW_CONTAINER_STATE_H_
