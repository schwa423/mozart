// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/tests/scene_manager_test.h"

#include "apps/mozart/lib/tests/test_with_message_loop.h"

namespace mozart {
namespace scene {
namespace test {

void SceneManagerTest::SetUp() {
  auto r = std::make_unique<ReleaseFenceSignallerForTest>(
      &command_buffer_sequencer_);
  auto session_context = std::make_unique<SessionContextForTest>(std::move(r));

  manager_impl_ =
      std::make_unique<SceneManagerImplForTest>(std::move(session_context));
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
}  // namespace scene
}  // namespace mozart
