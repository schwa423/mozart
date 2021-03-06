// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/examples/shadertoy/service/app.h"

#include "application/lib/app/application_context.h"
#include "apps/tracing/lib/trace/provider.h"
#include "escher/vk/vulkan_device_queues.h"
#include "escher/vk/vulkan_instance.h"

namespace shadertoy {

App::App(app::ApplicationContext* app_context, escher::Escher* escher)
    : factory_(std::make_unique<ShadertoyFactoryImpl>(this)),
      escher_(escher),
      renderer_(escher, kDefaultImageFormat),
      compiler_(escher,
                renderer_.render_pass(),
                renderer_.descriptor_set_layout()) {
  tracing::InitializeTracer(app_context, {"shadertoy"});

  app_context->outgoing_services()
      ->AddService<mozart::example::ShadertoyFactory>([this](
          fidl::InterfaceRequest<mozart::example::ShadertoyFactory> request) {
        FTL_LOG(INFO) << "Accepting connection to ShadertoyFactory";
        bindings_.AddBinding(factory_.get(), std::move(request));
      });
}

App::~App() = default;

}  // namespace shadertoy
