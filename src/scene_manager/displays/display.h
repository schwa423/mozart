// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>

#include "lib/ftl/macros.h"

namespace scene_manager {

// Display is a placeholder that provides make-believe values for screen
// resolution, vsync interval, last vsync time, etc.
class Display {
 public:
  // TODO(MZ-124): We should derive an appropriate value from the rendering
  // targets, in particular giving priority to couple to the display refresh
  // (vsync).
  static constexpr uint64_t kHardcodedPresentationIntervalNanos = 16'666'667;

  Display(uint32_t width, uint32_t height, float device_pixel_ratio);

  // Obtain the time of the last Vsync, in nanoseconds.
  uint64_t GetLastVsyncTime() const;

  // Obtain the interval between Vsyncs.
  uint64_t GetVsyncInterval() const;

  // Claiming a display means that no other display renderer can use it.
  bool is_claimed() const { return claimed_; }
  void Claim();
  void Unclaim();

  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  float device_pixel_ratio() const { return device_pixel_ratio_; }

 private:
  uint64_t const first_vsync_;
  uint32_t const width_;
  uint32_t const height_;
  float const device_pixel_ratio_;

  bool claimed_ = false;

  FTL_DISALLOW_COPY_AND_ASSIGN(Display);
};

}  // namespace scene_manager
