// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_INPUT_MANAGER_INPUT_ASSOCIATE_H_
#define APPS_MOZART_SRC_INPUT_MANAGER_INPUT_ASSOCIATE_H_

#include <memory>
#include <unordered_map>

#include "apps/mozart/lib/view_associate_framework/view_inspector_client.h"
#include "apps/mozart/services/views/view_associates.fidl.h"
#include "apps/mozart/src/input_manager/input_connection_impl.h"
#include "apps/mozart/src/input_manager/input_dispatcher_impl.h"
#include "apps/mozart/src/input_manager/text_input_service_impl.h"
#include "lib/ftl/macros.h"

namespace input_manager {

// InputManager's ViewAssociate interface implementation.
class InputAssociate : public mozart::ViewAssociate {
 public:
  InputAssociate();
  ~InputAssociate() override;

  const ftl::RefPtr<mozart::ViewInspectorClient>& inspector() {
    return inspector_;
  }

  // |ViewAssociate|:
  void Connect(fidl::InterfaceHandle<mozart::ViewInspector> inspector,
               const ConnectCallback& callback) override;
  void ConnectToViewService(mozart::ViewTokenPtr view_token,
                            const fidl::String& service_name,
                            mx::channel client_handle) override;
  void ConnectToViewTreeService(mozart::ViewTreeTokenPtr view_tree_token,
                                const fidl::String& service_name,
                                mx::channel client_handle) override;

  // Delivers an event to a view.
  void DeliverEvent(const mozart::ViewToken* view_token,
                    mozart::InputEventPtr event,
                    OnEventDelivered callback);
  // Query view for hit test
  void ViewHitTest(const mozart::ViewToken* view_token,
                   mozart::PointFPtr point,
                   const mozart::ViewHitTester::HitTestCallback& callback);
  // Callbacks.
  void OnInputConnectionDied(InputConnectionImpl* connection);
  void OnInputDispatcherDied(InputDispatcherImpl* dispatcher);
  void OnTextInputServiceDied(TextInputServiceImpl* text_input_service);

 private:
  void CreateInputConnection(
      mozart::ViewTokenPtr view_token,
      fidl::InterfaceRequest<mozart::InputConnection> request);
  void CreateInputDispatcher(
      mozart::ViewTreeTokenPtr view_tree_token,
      fidl::InterfaceRequest<mozart::InputDispatcher> request);
  void CreateTextInputService(
      mozart::ViewTokenPtr view_token,
      fidl::InterfaceRequest<mozart::TextInputService> request);

  ftl::RefPtr<mozart::ViewInspectorClient> inspector_;
  std::unordered_map<uint32_t, std::unique_ptr<InputConnectionImpl>>
      input_connections_by_view_token_;
  std::unordered_map<uint32_t, std::unique_ptr<TextInputServiceImpl>>
      text_input_services_by_view_token_;
  std::unordered_map<uint32_t, std::unique_ptr<InputDispatcherImpl>>
      input_dispatchers_by_view_tree_token_;

  FTL_DISALLOW_COPY_AND_ASSIGN(InputAssociate);
};

}  // namespace input_manager

#endif  // APPS_MOZART_SRC_INPUT_MANAGER_INPUT_ASSOCIATE_H_
