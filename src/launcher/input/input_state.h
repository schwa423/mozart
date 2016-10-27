// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_LAUNCHER_INPUT_INPUT_STATE_H_
#define APPS_MOZART_SRC_LAUNCHER_INPUT_INPUT_STATE_H_

#include <stdint.h>

#include <vector>

#include <hid/hid.h>
#include <hid/usages.h>

#include "apps/mozart/services/input/interfaces/input_events.mojom.h"
#include "apps/mozart/src/launcher/input/input_report.h"

namespace mozart {
namespace input {

using OnEventCallback = std::function<void(mozart::EventPtr event)>;

class KeyboardState {
 public:
  KeyboardState();
  void Update(const KeyboardReport& report,
              const KeyboardDescriptor& descriptor,
              OnEventCallback callback);

 private:
  std::vector<KeyUsage> keys_;
  int modifiers_ = 0;
  keychar_t* keymap_;  // assigned to a global static qwerty_map or dvorak_map
};

class MouseState {
 public:
  void Update(const MouseReport& report,
              const MouseDescriptor& descriptor,
              OnEventCallback callback);

 private:
  void SendEvent(float rel_x,
                 float rel_y,
                 int64_t timestamp,
                 mozart::EventType type,
                 mozart::EventFlags flags,
                 OnEventCallback callback);

  uint8_t buttons_ = 0;
};

class StylusState {
 public:
  void Update(const StylusReport& report,
              const StylusDescriptor& descriptor,
              OnEventCallback callback);

 private:
  bool stylus_down_ = false;
  mozart::PointerData stylus_;
};

class TouchscreenState {
 public:
  void Update(const TouchReport& report,
              const TouchscreenDescriptor& descriptor,
              OnEventCallback callback);

 private:
  std::vector<mozart::PointerData> pointers_;
};

struct DeviceState {
  KeyboardState keyboard;
  MouseState mouse;
  StylusState stylus;
  TouchscreenState touchscreen;
};

}  // namespace input
}  // namespace mozart

#endif  // APPS_MOZART_SRC_LAUNCHER_INPUT_INPUT_STATE_H_
