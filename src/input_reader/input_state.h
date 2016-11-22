// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_INPUT_READER_INPUT_STATE_H_
#define APPS_MOZART_SRC_INPUT_READER_INPUT_STATE_H_

#include <hid/hid.h>
#include <hid/usages.h>
#include <stdint.h>

#include <vector>

#include "apps/mozart/services/input/input_events.fidl.h"
#include "apps/mozart/src/input_reader/input_report.h"
#include "lib/ftl/memory/ref_counted.h"
#include "lib/ftl/memory/weak_ptr.h"
#include "lib/mtl/tasks/message_loop.h"

namespace mozart {
namespace input {

using OnEventCallback = std::function<void(mozart::EventPtr event)>;

class KeyboardState {
 public:
  KeyboardState(OnEventCallback callback);
  void Update(const KeyboardReport& report,
              const KeyboardDescriptor& descriptor);

 private:
  void SendEvent(mozart::EventType type,
                 KeyUsage key,
                 uint64_t modifiers,
                 uint64_t timestamp,
                 bool is_repeat);
  void Repeat(uint64_t sequence);
  void ScheduleRepeat(uint64_t sequence, ftl::TimeDelta delta);

  OnEventCallback callback_;
  keychar_t* keymap_;  // assigned to a global static qwerty_map or dvorak_map
  ftl::WeakPtrFactory<KeyboardState> weak_ptr_factory_;
  ftl::RefPtr<ftl::TaskRunner> task_runner_;

  std::vector<KeyUsage> keys_;
  std::vector<KeyUsage> repeat_keys_;
  uint64_t modifiers_ = 0;
  uint64_t repeat_sequence_ = 0;
};

class MouseState {
 public:
  MouseState(OnEventCallback callback) : callback_(callback) {}
  void Update(const MouseReport& report,
              const MouseDescriptor& descriptor,
              mozart::Size display_size);

 private:
  void SendEvent(float rel_x,
                 float rel_y,
                 int64_t timestamp,
                 mozart::EventType type,
                 mozart::EventFlags flags);

  OnEventCallback callback_;
  uint8_t buttons_ = 0;
  mozart::PointF position_;
};

class StylusState {
 public:
  StylusState(OnEventCallback callback) : callback_(callback) {}
  void Update(const StylusReport& report,
              const StylusDescriptor& descriptor,
              mozart::Size display_size);

 private:
  OnEventCallback callback_;
  bool stylus_down_ = false;
  mozart::PointerData stylus_;
};

class TouchscreenState {
 public:
  TouchscreenState(OnEventCallback callback) : callback_(callback) {}
  void Update(const TouchReport& report,
              const TouchscreenDescriptor& descriptor,
              mozart::Size display_size);

 private:
  OnEventCallback callback_;
  std::vector<mozart::PointerData> pointers_;
};

struct DeviceState {
  DeviceState(OnEventCallback callback)
      : keyboard(callback),
        mouse(callback),
        stylus(callback),
        touchscreen(callback) {}

  KeyboardState keyboard;
  MouseState mouse;
  StylusState stylus;
  TouchscreenState touchscreen;
};

}  // namespace input
}  // namespace mozart

#endif  // APPS_MOZART_SRC_INPUT_READER_INPUT_STATE_H_
