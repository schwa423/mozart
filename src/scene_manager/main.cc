// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "lib/escher/examples/common/demo_harness_fuchsia.h"
#include "lib/ftl/command_line.h"
#include "lib/ftl/log_settings_command_line.h"
#include "lib/ftl/logging.h"
#include "lib/mtl/tasks/message_loop.h"

#include "apps/mozart/src/scene_manager/displays/display_manager.h"
#include "apps/mozart/src/scene_manager/scene_manager_app.h"

using namespace scene_manager;

int main(int argc, const char** argv) {
  auto command_line = ftl::CommandLineFromArgcArgv(argc, argv);
  if (!ftl::SetLogSettingsFromCommandLine(command_line))
    return 1;

  SceneManagerApp::Params params;
  if (!params.Setup(command_line))
    return 1;

  mtl::MessageLoop loop;

  std::unique_ptr<SceneManagerApp> scene_manager_app;

  // Don't initialize Vulkan and the SceneManagerApp until display is ready.
  DisplayManager display_manager;
  display_manager.WaitForDefaultDisplay([&scene_manager_app, &params,
                                         &display_manager]() {
    Display* display = display_manager.default_display();
    FTL_CHECK(display) << "No default display available.";

    auto harness = DemoHarness::New(
        DemoHarness::WindowParams{"Mozart SceneManager", display->width(),
                                  display->height(), 2, false},
        DemoHarness::InstanceParams());

    app::ApplicationContext* application_context =
        static_cast<DemoHarnessFuchsia*>(harness.get())->application_context();
    scene_manager_app = std::make_unique<SceneManagerApp>(
        application_context, &params, &display_manager, std::move(harness));
  });

  loop.Run();
  return 0;
}
