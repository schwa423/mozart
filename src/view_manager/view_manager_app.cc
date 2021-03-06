// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/view_manager/view_manager_app.h"

#include "apps/mozart/src/view_manager/view_manager_impl.h"
#include "apps/tracing/lib/trace/provider.h"
#include "lib/ftl/logging.h"

namespace view_manager {

ViewManagerApp::ViewManagerApp()
    : application_context_(app::ApplicationContext::CreateFromStartupInfo()) {
  FTL_DCHECK(application_context_);

  tracing::InitializeTracer(application_context_.get(), {"view_manager"});

  registry_.reset(new ViewRegistry(application_context_.get()));

  application_context_->outgoing_services()->AddService<mozart::ViewManager>(
      [this](fidl::InterfaceRequest<mozart::ViewManager> request) {
        view_manager_bindings_.AddBinding(
            std::make_unique<ViewManagerImpl>(registry_.get()),
            std::move(request));
      });
}

ViewManagerApp::~ViewManagerApp() {}

}  // namespace view_manager
