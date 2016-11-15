// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_COMPOSITOR_BACKEND_FRAMEBUFFER_OUTPUT_H_
#define APPS_MOZART_SRC_COMPOSITOR_BACKEND_FRAMEBUFFER_OUTPUT_H_

#include <thread>

#include "apps/mozart/src/compositor/backend/output.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/ref_counted.h"
#include "lib/ftl/memory/weak_ptr.h"
#include "lib/ftl/tasks/task_runner.h"
#include "lib/ftl/time/time_delta.h"
#include "lib/ftl/time/time_point.h"

namespace compositor {

// Renderer backed by a Framebuffer on a new virtual console.
class FramebufferOutput : public Output {
 public:
  FramebufferOutput();
  ~FramebufferOutput() override;

  void Initialize(ftl::Closure error_callback);

  // |Output|:
  void GetDisplayInfo(DisplayCallback callback) override;
  void ScheduleFrame(FrameCallback callback) override;
  void SubmitFrame(ftl::RefPtr<RenderFrame> frame) override;

 private:
  class Rasterizer;

  void PostErrorCallback();
  void PostFrameToRasterizer(ftl::RefPtr<RenderFrame> frame);

  void OnDisplayReady(mozart::DisplayInfoPtr display_info);

  void OnFrameFinished(uint32_t frame_number,
                       ftl::TimePoint submit_time,
                       ftl::TimePoint start_time,
                       ftl::TimePoint finish_time);
  void PrepareNextFrame();
  void RunScheduledFrameCallback();

  ftl::RefPtr<ftl::TaskRunner> compositor_task_runner_;
  ftl::Closure error_callback_;

  std::unique_ptr<Rasterizer> rasterizer_;
  std::thread rasterizer_thread_;
  ftl::RefPtr<ftl::TaskRunner> rasterizer_task_runner_;

  FrameCallback scheduled_frame_callback_;

  uint32_t frame_number_ = 0u;
  bool frame_in_progress_ = true;  // wait for display ready
  ftl::RefPtr<RenderFrame> next_frame_;

  ftl::TimePoint last_submit_time_;
  ftl::TimePoint last_presentation_time_;
  ftl::TimeDelta presentation_latency_;

  mozart::DisplayInfoPtr display_info_;
  std::vector<DisplayCallback> display_callbacks_;

  ftl::WeakPtrFactory<FramebufferOutput> weak_ptr_factory_;

  FTL_DISALLOW_COPY_AND_ASSIGN(FramebufferOutput);
};

}  // namespace compositor

#endif  // APPS_MOZART_SRC_COMPOSITOR_BACKEND_FRAMEBUFFER_OUTPUT_H_
