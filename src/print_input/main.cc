// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/services/input/cpp/formatting.h"
#include "apps/mozart/src/input_reader/input_reader.h"
#include "lib/ftl/command_line.h"
#include "lib/ftl/logging.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/strings/string_printf.h"
#include "lib/mtl/tasks/message_loop.h"

int main(int argc, char** argv) {
  mtl::MessageLoop message_loop;

  mozart::input::InputInterpreter interpreter;
  mozart::Size size;
  size.width = 1.0;
  size.height = 1.0;
  interpreter.RegisterDisplay(size);
  interpreter.RegisterCallback(
      [](mozart::InputEventPtr event) { FTL_LOG(INFO) << *(event.get()); });
  mozart::input::InputReader reader(&interpreter);
  reader.Start();

  message_loop.Run();
  return 0;
}
