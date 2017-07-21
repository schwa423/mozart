// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene_manager/tests/scene_manager_test.h"

#include "apps/mozart/lib/tests/test_with_message_loop.h"

namespace scene_manager {
namespace test {

void SceneManagerTest::SetUp() {
  auto r = std::make_unique<ReleaseFenceSignallerForTest>(
      &command_buffer_sequencer_);
  auto engine = std::make_unique<EngineForTest>(std::move(r));

  display_ = std::make_unique<Display>(1280, 800, 1.f);
  manager_impl_ = std::make_unique<SceneManagerImplForTest>(display_.get(),
                                                            std::move(engine));

  manager_binding_ = std::make_unique<fidl::Binding<mozart2::SceneManager>>(
      manager_impl_.get());

  thread_ = std::make_unique<mtl::Thread>();
  thread_->Run();

  auto interface_request = manager_.NewRequest();

  ftl::ManualResetWaitableEvent wait;
  thread_->TaskRunner()->PostTask([this, &interface_request, &wait]() {
    this->manager_binding_->Bind(std::move(interface_request));
    this->manager_binding_->set_connection_error_handler(
        [this]() { this->manager_impl_.reset(); });
    wait.Signal();
  });
  wait.Wait();
}

void SceneManagerTest::TearDown() {
  manager_ = nullptr;
  RUN_MESSAGE_LOOP_UNTIL(manager_impl_ == nullptr);
  thread_->TaskRunner()->PostTask(
      []() { mtl::MessageLoop::GetCurrent()->QuitNow(); });
  thread_->Join();
}

}  // namespace test
}  // namespace scene_manager
