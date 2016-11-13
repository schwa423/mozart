// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/launcher/app.h"

#include <algorithm>

#include "apps/modular/lib/app/connect.h"
#include "apps/mozart/services/views/view_provider.fidl.h"
#include "apps/mozart/src/launcher/presenter.h"
#include "lib/ftl/logging.h"

namespace launcher {

App::App(const ftl::CommandLine& command_line)
    : application_context_(
          modular::ApplicationContext::CreateFromStartupInfo()) {
  FTL_DCHECK(application_context_);

  // Register launcher service.
  application_context_->outgoing_services()->AddService<mozart::Launcher>(
      [this](fidl::InterfaceRequest<mozart::Launcher> request) {
        launcher_bindings_.AddBinding(this, std::move(request));
      });

  // Launch program with arguments supplied on command-line.
  const auto& positional_args = command_line.positional_args();
  if (!positional_args.empty()) {
    fidl::String url = positional_args[0];
    fidl::Array<fidl::String> arguments;
    for (size_t i = 1; i < positional_args.size(); ++i)
      arguments.push_back(positional_args[i]);
    Launch(std::move(url), std::move(arguments));
  }
}

App::~App() {}

void App::Launch(fidl::String url, fidl::Array<fidl::String> arguments) {
  FTL_LOG(INFO) << "Launching view provider " << url;

  modular::ServiceProviderPtr services;
  modular::ApplicationControllerPtr controller;
  auto launch_info = modular::ApplicationLaunchInfo::New();
  launch_info->url = std::move(url);
  launch_info->arguments = std::move(arguments);
  launch_info->services = GetProxy(&services);
  application_context_->launcher()->CreateApplication(
      std::move(launch_info), GetProxy(&app_controller_));
  app_controller_.set_connection_error_handler([] {
    FTL_LOG(INFO) << "Launched application terminated.";
    exit(0);
  });

  fidl::InterfacePtr<mozart::ViewProvider> view_provider;
  modular::ConnectToService(services.get(), GetProxy(&view_provider));

  fidl::InterfaceHandle<mozart::ViewOwner> view_owner;
  view_provider->CreateView(fidl::GetProxy(&view_owner), nullptr);

  Display(std::move(view_owner));
}

void App::Display(fidl::InterfaceHandle<mozart::ViewOwner> view_owner_handle) {
  mozart::ViewOwnerPtr view_owner =
      mozart::ViewOwnerPtr::Create(std::move(view_owner_handle));

  InitializeServices();

  auto launch = std::make_unique<Presenter>(
      compositor_.get(), view_manager_.get(), std::move(view_owner));
  launch->set_shutdown_callback([ this, launch = launch.get() ] {
    auto it = std::find_if(presenters_.begin(), presenters_.end(),
                           [launch](const std::unique_ptr<Presenter>& other) {
                             return other.get() == launch;
                           });
    FTL_DCHECK(it != presenters_.end());
    presenters_.erase(it);
  });
  launch->Show();
  presenters_.push_back(std::move(launch));
}

void App::InitializeServices() {
  if (!compositor_) {
    application_context_->ConnectToEnvironmentService(GetProxy(&compositor_));
    compositor_.set_connection_error_handler([this] {
      FTL_LOG(ERROR) << "Compositor died, destroying view trees.";
      Reset();
    });
  }

  if (!view_manager_) {
    application_context_->ConnectToEnvironmentService(GetProxy(&view_manager_));
    view_manager_.set_connection_error_handler([this] {
      FTL_LOG(ERROR) << "ViewManager died, destroying view trees.";
      Reset();
    });
  }
}

void App::Reset() {
  presenters_.clear();  // must be first, holds pointers to services
  compositor_.reset();
  view_manager_.reset();
}

}  // namespace launcher
