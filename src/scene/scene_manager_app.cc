// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/scene_manager_app.h"

#include "apps/tracing/lib/trace/provider.h"
#include "lib/ftl/logging.h"

namespace mozart {
namespace scene {

SceneManagerApp::SceneManagerApp(app::ApplicationContext* app_context,
                                 Params* params,
                                 std::unique_ptr<DemoHarness> demo_harness)
    : application_context_(app_context),
      demo_harness_(std::move(demo_harness)),
      vulkan_context_(demo_harness_->GetVulkanContext()),
      escher_(vulkan_context_),
      scene_manager_(std::make_unique<SceneManagerImpl>(
          &escher_,
          std::make_unique<FrameScheduler>(&display_),
          std::make_unique<escher::VulkanSwapchain>(
              demo_harness_->GetVulkanSwapchain()))) {
  FTL_DCHECK(application_context_);

  tracing::InitializeTracer(application_context_, {"scene_manager"});

  application_context_->outgoing_services()->AddService<mozart2::SceneManager>(
      [this](fidl::InterfaceRequest<mozart2::SceneManager> request) {
        FTL_LOG(INFO) << "Accepting connection to SceneManagerImpl";
        bindings_.AddBinding(scene_manager_.get(), std::move(request));
      });
}

SceneManagerApp::~SceneManagerApp() {}

}  // namespace scene
}  // namespace mozart
