// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "lib/ftl/synchronization/waitable_event.h"

#include "apps/mozart/lib/scene/session_helpers.h"
#include "apps/mozart/lib/tests/test_with_message_loop.h"
#include "apps/mozart/src/scene_manager/resources/nodes/entity_node.h"
#include "apps/mozart/src/scene_manager/tests/mocks.h"
#include "apps/mozart/src/scene_manager/tests/scene_manager_test.h"
#include "apps/mozart/src/scene_manager/tests/util.h"

namespace scene_manager {
namespace test {

TEST_F(SceneManagerTest, CreateAndDestroySession) {
  mozart2::SessionPtr session;
  EXPECT_EQ(0U, engine()->GetSessionCount());
  manager_->CreateSession(session.NewRequest(), nullptr);
  RUN_MESSAGE_LOOP_UNTIL(engine()->GetSessionCount() == 1);
  session = nullptr;
  RUN_MESSAGE_LOOP_UNTIL(engine()->GetSessionCount() == 0);
}

bool IsFenceSignalled(const mx::event& fence) {
  mx_signals_t signals = 0u;
  mx_status_t status = fence.wait_one(kFenceSignalledOrClosed, 0, &signals);
  FTL_DCHECK(status == MX_OK || status == MX_ERR_TIMED_OUT);
  return signals & kFenceSignalledOrClosed;
}

TEST_F(SceneManagerTest, ReleaseFences) {
  // Tests creating a session, and calling Present with two release fences.
  // The release fences should be signalled after a subsequent Present.
  EXPECT_EQ(0u, engine()->GetSessionCount());

  mozart2::SessionPtr session;
  manager_->CreateSession(session.NewRequest(), nullptr);

  RUN_MESSAGE_LOOP_UNTIL(engine()->GetSessionCount() == 1);
  EXPECT_EQ(1u, engine()->GetSessionCount());
  auto handler = static_cast<SessionHandlerForTest*>(engine()->FindSession(1));

  {
    ::fidl::Array<mozart2::OpPtr> ops;
    ops.push_back(mozart::NewCreateCircleOp(1, 50.f));
    ops.push_back(mozart::NewCreateCircleOp(2, 25.f));
    session->Enqueue(std::move(ops));
  }
  RUN_MESSAGE_LOOP_UNTIL(handler->enqueue_count() == 1);
  EXPECT_EQ(1u, handler->enqueue_count());

  // Create release fences
  mx::event release_fence1;
  ASSERT_EQ(MX_OK, mx::event::create(0, &release_fence1));
  mx::event release_fence2;
  ASSERT_EQ(MX_OK, mx::event::create(0, &release_fence2));

  ::fidl::Array<mx::event> release_fences;
  release_fences.push_back(CopyEvent(release_fence1));
  release_fences.push_back(CopyEvent(release_fence2));

  EXPECT_FALSE(IsFenceSignalled(release_fence1));
  EXPECT_FALSE(IsFenceSignalled(release_fence2));

  // Call Present with release fences.
  session->Present(0u, ::fidl::Array<mx::event>::New(0),
                   std::move(release_fences),
                   [](mozart2::PresentationInfoPtr info) {});
  RUN_MESSAGE_LOOP_UNTIL(handler->present_count() == 1);
  EXPECT_EQ(1u, handler->present_count());

  EXPECT_FALSE(IsFenceSignalled(release_fence1));
  EXPECT_FALSE(IsFenceSignalled(release_fence2));
  // Call Present again with no release fences.
  session->Present(0u, ::fidl::Array<mx::event>::New(0),
                   ::fidl::Array<mx::event>::New(0),
                   [](mozart2::PresentationInfoPtr info) {});
  RUN_MESSAGE_LOOP_UNTIL(handler->present_count() == 2);
  EXPECT_EQ(2u, handler->present_count());

  RUN_MESSAGE_LOOP_UNTIL(IsFenceSignalled(release_fence1));
  EXPECT_TRUE(IsFenceSignalled(release_fence2));
}

TEST_F(SceneManagerTest, AcquireAndReleaseFences) {
  // Tests creating a session, and calling Present with an acquire and a release
  // fence. The release fences should be signalled only after a subsequent
  // Present, and not until the acquire fence has been signalled.
  EXPECT_EQ(0u, engine()->GetSessionCount());

  mozart2::SessionPtr session;
  manager_->CreateSession(session.NewRequest(), nullptr);

  RUN_MESSAGE_LOOP_UNTIL(engine()->GetSessionCount() == 1);
  EXPECT_EQ(1u, engine()->GetSessionCount());
  auto handler = static_cast<SessionHandlerForTest*>(engine()->FindSession(1));

  {
    ::fidl::Array<mozart2::OpPtr> ops;
    ops.push_back(mozart::NewCreateCircleOp(1, 50.f));
    ops.push_back(mozart::NewCreateCircleOp(2, 25.f));
    session->Enqueue(std::move(ops));
  }
  RUN_MESSAGE_LOOP_UNTIL(handler->enqueue_count() == 1);
  EXPECT_EQ(1u, handler->enqueue_count());

  // Create acquire and release fences
  mx::event acquire_fence;
  ASSERT_EQ(MX_OK, mx::event::create(0, &acquire_fence));
  mx::event release_fence;
  ASSERT_EQ(MX_OK, mx::event::create(0, &release_fence));

  ::fidl::Array<mx::event> acquire_fences;
  acquire_fences.push_back(CopyEvent(acquire_fence));

  ::fidl::Array<mx::event> release_fences;
  release_fences.push_back(CopyEvent(release_fence));

  // Call Present with both the acquire and release fences.
  session->Present(0u, std::move(acquire_fences), std::move(release_fences),
                   [](mozart2::PresentationInfoPtr info) {});
  RUN_MESSAGE_LOOP_UNTIL(handler->present_count() == 1);
  EXPECT_EQ(1u, handler->present_count());

  EXPECT_FALSE(IsFenceSignalled(release_fence));

  // Call Present again with no fences.
  session->Present(0u, ::fidl::Array<mx::event>::New(0),
                   ::fidl::Array<mx::event>::New(0),
                   [](mozart2::PresentationInfoPtr info) {});
  RUN_MESSAGE_LOOP_UNTIL(handler->present_count() == 2);

  EXPECT_FALSE(IsFenceSignalled(release_fence));

  // Now signal the acquire fence.
  acquire_fence.signal(0u, kFenceSignalled);

  // Now expect that the first frame was presented, and its release fence was
  // signalled.
  RUN_MESSAGE_LOOP_UNTIL(IsFenceSignalled(release_fence));
}

}  // namespace test
}  // namespace scene_manager
