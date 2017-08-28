// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/examples/shadertoy/client/view.h"
#include "apps/mozart/lib/view_framework/view_provider_app.h"
#include "lib/ftl/command_line.h"
#include "lib/ftl/log_settings_command_line.h"
#include "lib/mtl/tasks/message_loop.h"

int main(int argc, const char** argv) {
  auto command_line = ftl::CommandLineFromArgcArgv(argc, argv);
  if (!ftl::SetLogSettingsFromCommandLine(command_line))
    return 1;

  mtl::MessageLoop loop;

  mozart::ViewProviderApp app([](mozart::ViewContext view_context) {
    return std::make_unique<shadertoy_client::View>(
        view_context.application_context, std::move(view_context.view_manager),
        std::move(view_context.view_owner_request));
  });

  loop.Run();
  return 0;
}
