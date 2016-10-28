// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/services/composition/interfaces/compositor.mojom-sync.h"
#include "apps/mozart/services/composition/interfaces/scheduling.mojom-sync.h"
#include "lib/ftl/macros.h"
#include "mojo/public/cpp/application/application_test_base.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/cpp/bindings/synchronous_interface_ptr.h"
#include "mojo/services/gpu/interfaces/context_provider.mojom.h"
#include "mojo/services/native_viewport/interfaces/native_viewport.mojom.h"

namespace test {

using SynchronousCompositorPtr = mojo::SynchronousInterfacePtr<Compositor>;

using SynchronousFrameSchedulerPtr =
    mojo::SynchronousInterfacePtr<FrameScheduler>;

class SchedulingTest : public mojo::test::ApplicationTestBase {
 public:
  SchedulingTest() {}

 protected:
  void SetUp() override {
    mojo::test::ApplicationTestBase::SetUp();

    mojo::ConnectToService(shell(), "mojo:native_viewport_service",
                           GetProxy(&viewport_));
    auto size = mojo::Size::New();
    size->width = 320;
    size->height = 640;
    auto configuration = mojo::SurfaceConfiguration::New();
    viewport_->Create(size.Pass(), configuration.Pass(),
                      [](mojo::ViewportMetricsPtr metrics) {

                      });
    viewport_->Show();

    mojo::ContextProviderPtr context_provider;
    viewport_->GetContextProvider(GetProxy(&context_provider));

    mojo::ConnectToService(shell(), "mojo:compositor_service",
                           mojo::GetSynchronousProxy(&compositor_));
    compositor_->CreateRenderer(context_provider.Pass(),
                                mojo::GetProxy(&renderer_), "SchedulingTest");
  }

  void TestScheduler(SynchronousFrameSchedulerPtr scheduler) {
    FrameInfoPtr frame_info1;
    ASSERT_TRUE(scheduler->ScheduleFrame(&frame_info1));
    AssertValidFrameInfo(frame_info1.get());

    FrameInfoPtr frame_info2;
    ASSERT_TRUE(scheduler->ScheduleFrame(&frame_info2));
    AssertValidFrameInfo(frame_info2.get());

    EXPECT_GT(frame_info2->base_time, frame_info1->base_time);
    EXPECT_GT(frame_info2->presentation_time, frame_info1->presentation_time);
  }

  void AssertValidFrameInfo(FrameInfo* frame_info) {
    ASSERT_NE(nullptr, frame_info);
    EXPECT_LT(frame_info->base_time, MojoGetTimeTicksNow());
    EXPECT_GT(frame_info->presentation_interval, 0u);
    EXPECT_GT(frame_info->publish_deadline, frame_info->base_time);
    EXPECT_GT(frame_info->presentation_time, frame_info->publish_deadline);
  }

  mojo::NativeViewportPtr viewport_;
  SynchronousCompositorPtr compositor_;
  RendererPtr renderer_;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(SchedulingTest);
};

namespace {

TEST_F(SchedulingTest, RendererScheduler) {
  SynchronousFrameSchedulerPtr scheduler;
  renderer_->GetScheduler(mojo::GetSynchronousProxy(&scheduler));
  TestScheduler(scheduler.Pass());
}

// Test what happens when a scene is not attached to a renderer.
// It should still receive scheduled frame updates occasionally albeit
// at some indeterminate rate (enough to keep the scene from hanging).
TEST_F(SchedulingTest, OrphanedSceneScheduler) {
  ScenePtr scene;
  SceneTokenPtr scene_token;
  compositor_->CreateScene(mojo::GetProxy(&scene), "SchedulingTest",
                           &scene_token);

  SynchronousFrameSchedulerPtr scheduler;
  scene->GetScheduler(mojo::GetSynchronousProxy(&scheduler));
  TestScheduler(scheduler.Pass());
}

// Test what happens when a scene is attached to a renderer.
// It should receive scheduled frame updates at a rate determined
// by the renderer.
TEST_F(SchedulingTest, RootSceneScheduler) {
  ScenePtr scene;
  SceneTokenPtr scene_token;
  compositor_->CreateScene(mojo::GetProxy(&scene), "SchedulingTest",
                           &scene_token);

  auto viewport = mojo::Rect::New();
  viewport->width = 1;
  viewport->height = 1;
  renderer_->SetRootScene(scene_token.Pass(), kSceneVersionNone,
                          viewport.Pass());

  SynchronousFrameSchedulerPtr scheduler;
  scene->GetScheduler(mojo::GetSynchronousProxy(&scheduler));
  TestScheduler(scheduler.Pass());
}

}  // namespace
}  // namespace mozart
