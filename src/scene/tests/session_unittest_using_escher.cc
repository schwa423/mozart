// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/lib/app/application_context.h"
#include "apps/mozart/lib/tests/test_with_message_loop.h"
#include "apps/mozart/src/scene/tests/escher_test_environment.h"
#include "apps/mozart/src/scene/tests/session_test.h"
#include "apps/test_runner/lib/reporting/gtest_listener.h"
#include "apps/test_runner/lib/reporting/reporter.h"
#include "apps/test_runner/lib/reporting/results_queue.h"
#include "escher/examples/common/demo_harness.h"
#include "gtest/gtest.h"
#include "lib/mtl/threading/thread.h"

std::unique_ptr<mozart::scene::test::EscherTestEnvironment> g_escher_env;

int main(int argc, char** argv) {
  // Add a global environment which will set up (and tear down) DemoHarness and
  // Escher. This also implicitly creates a message loop.
  g_escher_env = std::make_unique<mozart::scene::test::EscherTestEnvironment>();
  g_escher_env->SetUp(argv[0]);

  // TestRunner setup. Copied from //apps/test_runner/src/gtest_main.cc.
  mtl::Thread reporting_thread;

  test_runner::ResultsQueue queue;
  test_runner::Reporter reporter(argv[0], &queue);
  test_runner::GTestListener listener(argv[0], &queue);

  reporting_thread.Run();
  reporting_thread.TaskRunner()->PostTask([&reporter] {
    auto context = app::ApplicationContext::CreateFromStartupInfoNotChecked();
    reporter.Start(context.get());
  });

  testing::InitGoogleTest(&argc, argv);
  testing::UnitTest::GetInstance()->listeners().Append(&listener);
  int status = RUN_ALL_TESTS();
  testing::UnitTest::GetInstance()->listeners().Release(&listener);

  reporting_thread.Join();
  g_escher_env->TearDown();
  return status;
}
