// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/ime/app.h"

#include <algorithm>

#include "application/lib/app/connect.h"
#include "apps/mozart/services/input/cpp/formatting.h"
#include "apps/mozart/services/input/text_input.fidl.h"
#include "apps/mozart/src/ime/ime_impl.h"
#include "lib/ftl/logging.h"

namespace ime {

App::App(const ftl::CommandLine& command_line)
    : application_context_(app::ApplicationContext::CreateFromStartupInfo()) {
  FTL_DCHECK(application_context_);
  application_context_->outgoing_services()->AddService<mozart::ImeService>(
      [this](fidl::InterfaceRequest<mozart::ImeService> request) {
        ime_bindings_.AddBinding(this, std::move(request));
      });
}

App::~App() {}

void App::GetInputMethodEditor(
    mozart::KeyboardType keyboard_type,
    mozart::TextInputStatePtr initial_state,
    fidl::InterfaceHandle<mozart::InputMethodEditorClient> client,
    fidl::InterfaceRequest<mozart::InputMethodEditor> editor_request) {
  FTL_DCHECK(initial_state);
  FTL_DCHECK(client);
  FTL_DCHECK(editor_request.is_pending());

  FTL_VLOG(1) << "GetInputMethodEditor: "
              << ", keyboard_type=" << keyboard_type
              << ", initial_state=" << *initial_state;

  std::unique_ptr<ImeImpl> ime_impl =
      std::make_unique<ImeImpl>(keyboard_type, std::move(initial_state),
                                std::move(client), std::move(editor_request));
  // FIXME(jpoichet) we're leaking
  ime_.emplace(ime_.end(), std::move(ime_impl));
}

void App::InjectInput(mozart::InputEventPtr event) {
  FTL_VLOG(1) << "InjectInput: event=" << *event;
  for (auto& it : ime_) {
    it->InjectInput(event.Clone());
  }
}

void App::OnImeDisconnected(ImeImpl* ime) {}

}  // namespace ime
