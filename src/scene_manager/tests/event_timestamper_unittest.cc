// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "apps/mozart/lib/tests/test_with_message_loop.h"
#include "apps/mozart/src/scene_manager/fence.h"
#include "apps/mozart/src/scene_manager/tests/util.h"
#include "apps/mozart/src/scene_manager/util/event_timestamper.h"

namespace scene_manager {
namespace test {

TEST(EventTimestamper, SmokeTest) {
  constexpr size_t kEventCount = 3;
  std::vector<mx::event> events;
  std::vector<EventTimestamper::Watch> watches;
  std::vector<mx_time_t> target_callback_times;

  events.resize(kEventCount);
  watches.reserve(kEventCount);
  target_callback_times.resize(kEventCount);

  auto timestamper = std::make_unique<EventTimestamper>();

  for (size_t i = 0; i < kEventCount; ++i) {
    ASSERT_EQ(MX_OK, mx::event::create(0u, &(events[i])));
    watches.emplace_back(timestamper.get(), CopyEvent(events[i]),
                         MX_EVENT_SIGNALED, [&, i = i ](mx_time_t timestamp) {
                           ASSERT_GT(kEventCount, i);
                           EXPECT_GT(target_callback_times[i], 0u);
                           EXPECT_LE(target_callback_times[i], timestamp);
                           target_callback_times[i] = 0;
                         });
    target_callback_times[i] = 0;
  }

  for (size_t i = 0; i < kEventCount; ++i) {
    target_callback_times[i] = mx_time_get(MX_CLOCK_MONOTONIC);
    events[i].signal(0u, MX_EVENT_SIGNALED);
    watches[i].Start();
  }

  for (size_t i = 0; i < kEventCount; ++i) {
    RUN_MESSAGE_LOOP_UNTIL(target_callback_times[i] == 0u);
  }

  // Watches must not outlive the timestamper.
  watches.clear();
  timestamper.reset();
}

}  // namespace test
}  // namespace scene_manager
