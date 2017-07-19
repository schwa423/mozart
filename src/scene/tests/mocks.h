// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene/release_fence_signaller.h"
#include "apps/mozart/src/scene/scene_manager_impl.h"
#include "apps/mozart/src/scene/session/session_context.h"
#include "apps/mozart/src/scene/session/session_handler.h"

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

// Subclass SceneManagerImpl to make testing easier.
class SceneManagerImplForTest : public SceneManagerImpl {
 public:
  SceneManagerImplForTest(Display* display,
                          std::unique_ptr<SessionContext> session_context);
};

class ReleaseFenceSignallerForTest : public ReleaseFenceSignaller {
 public:
  ReleaseFenceSignallerForTest(
      escher::impl::CommandBufferSequencer* command_buffer_sequencer);

  void AddCPUReleaseFence(mx::event fence) override;

  uint32_t num_calls_to_add_cpu_release_fence() {
    return num_calls_to_add_cpu_release_fence_;
  }

 private:
  uint32_t num_calls_to_add_cpu_release_fence_ = 0;
};

class SessionContextForTest : public SessionContext {
 public:
  SessionContextForTest(std::unique_ptr<ReleaseFenceSignaller> r);
  using SessionContext::FindSession;

 private:
  std::unique_ptr<SessionHandler> CreateSessionHandler(
      SessionId id,
      ::fidl::InterfaceRequest<mozart2::Session> request,
      ::fidl::InterfaceHandle<mozart2::SessionListener> listener) override;
};

}  // namespace test
}  // namespace scene
}  // namespace mozart
