// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/view_manager/view_manager_app.h"

#include "application/lib/app/connect.h"
#include "apps/mozart/src/view_manager/view_manager_impl.h"
#include "apps/mozart/src/view_manager/view_registry1.h"
#include "apps/mozart/src/view_manager/view_registry2.h"
#include "apps/tracing/lib/trace/provider.h"
#include "lib/ftl/logging.h"

namespace view_manager {

ViewManagerApp::ViewManagerApp(Params* params)
    : application_context_(app::ApplicationContext::CreateFromStartupInfo()) {
  FTL_DCHECK(application_context_);

  tracing::InitializeTracer(application_context_.get(), {"view_manager"});

  if (params->use_composer2()) {
    mozart2::ComposerPtr composer =
        application_context_->ConnectToEnvironmentService<mozart2::Composer>();
    composer.set_connection_error_handler([] {
      FTL_LOG(ERROR) << "Exiting due to composer connection error.";
      exit(1);
    });
    registry_.reset(
        new ViewRegistry2(application_context_.get(), std::move(composer)));
  } else {
    mozart::CompositorPtr compositor =
        application_context_->ConnectToEnvironmentService<mozart::Compositor>();
    compositor.set_connection_error_handler([] {
      FTL_LOG(ERROR) << "Exiting due to compositor connection error.";
      exit(1);
    });
    registry_.reset(
        new ViewRegistry1(application_context_.get(), std::move(compositor)));
  }

  application_context_->outgoing_services()->AddService<mozart::ViewManager>(
      [this](fidl::InterfaceRequest<mozart::ViewManager> request) {
        view_manager_bindings_.AddBinding(
            std::make_unique<ViewManagerImpl>(registry_.get()),
            std::move(request));
      });
}

ViewManagerApp::~ViewManagerApp() {}

}  // namespace view_manager
