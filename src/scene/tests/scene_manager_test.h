// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/lib/tests/test_with_message_loop.h"
#include "apps/mozart/src/scene/scene_manager_impl.h"

#include "gtest/gtest.h"

namespace mozart {
namespace scene {
namespace test {

// Subclass SessionHandler to make testing easier.
class SessionHandlerForTest : public SessionHandler {
 public:
  SessionHandlerForTest(
      SessionContext* session_context,
      SessionId session_id,
      ::fidl::InterfaceRequest<mozart2::Session> request,
      ::fidl::InterfaceHandle<mozart2::SessionListener> listener);

  // mozart2::Session interface methods.
  void Enqueue(::fidl::Array<mozart2::OpPtr> ops) override;
  void Present(uint64_t presentation_time,
               ::fidl::Array<mx::event> acquire_fences,
               ::fidl::Array<mx::event> release_fences,
               const PresentCallback& callback) override;
  void Connect(
      ::fidl::InterfaceRequest<mozart2::Session> session,
      ::fidl::InterfaceHandle<mozart2::SessionListener> listener) override;

  // Return the number of Enqueue()/Present()/Connect() messages that have
  // been processed.
  uint32_t enqueue_count() const { return enqueue_count_; }
  uint32_t present_count() const { return present_count_; }
  uint32_t connect_count() const { return connect_count_; }

 private:
  std::atomic<uint32_t> enqueue_count_;
  std::atomic<uint32_t> present_count_;
  std::atomic<uint32_t> connect_count_;
};

// Subclass of SessionContext that exposes a default constructor. Leaves
// lots of instance variables uninitialized.
class SessionContextForTest : public SessionContext {
 public:
  SessionContextForTest();
  using SessionContext::FindSession;

 private:
  std::unique_ptr<SessionHandler> CreateSessionHandler(
      SessionId id,
      ::fidl::InterfaceRequest<mozart2::Session> request,
      ::fidl::InterfaceHandle<mozart2::SessionListener> listener) override;
};

// Subclass SceneManagerImpl to make testing easier.
class SceneManagerImplForTest : public SceneManagerImpl {
 public:
  SceneManagerImplForTest();
};

class SceneManagerTest : public ::testing::Test {
 public:
  // ::testing::Test virtual method.
  void SetUp() override;

  // ::testing::Test virtual method.
  void TearDown() override;

  SessionPtr NewSession();

 protected:
  mozart2::SceneManagerPtr manager_;
  std::unique_ptr<fidl::Binding<mozart2::SceneManager>> manager_binding_;
  std::unique_ptr<SceneManagerImplForTest> manager_impl_;
  std::unique_ptr<mtl::Thread> thread_;
};

}  // namespace test
}  // namespace scene
}  // namespace mozart
