// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/input_reader/input_state.h"

#include "lib/ftl/logging.h"
#include "lib/ftl/time/time_point.h"

namespace {
// The input event fidl is currently defined to expect some number
// of milliseconds.
int64_t InputEventTimestampNow() {
  return ftl::TimePoint::Now().ToEpochDelta().ToMilliseconds();
}
}  // namespace

namespace mozart {
namespace input {

constexpr uint32_t kMouseLeftButtonMask = 0x01;
constexpr uint32_t kMouseRightButtonMask = 0x02;
constexpr uint32_t kMouseMiddleButtonMask = 0x04;

#pragma mark - KeyboardState

KeyboardState::KeyboardState() : keymap_(qwerty_map) {
  char* keys = getenv("gfxconsole.keymap");
  if (keys && !strcmp(keys, "dvorak")) {
    keymap_ = dvorak_map;
  }
}

void KeyboardState::Update(const KeyboardReport& report,
                           const KeyboardDescriptor& descriptor,
                           OnEventCallback callback) {
  uint64_t now = InputEventTimestampNow();

  std::vector<KeyUsage> old_keys = keys_;
  keys_.clear();

  for (KeyUsage key : report.down) {
    keys_.push_back(key);
    auto it = std::find(old_keys.begin(), old_keys.end(), key);
    if (it != old_keys.end()) {
      old_keys.erase(it);
      continue;
    }

    mozart::EventPtr ev = mozart::Event::New();
    ev->action = mozart::EventType::KEY_PRESSED;
    ev->flags = mozart::EventFlags::NONE;
    ev->time_stamp = now;

    ev->key_data = mozart::KeyData::New();
    ev->key_data->hid_usage = key;
    ev->key_data->code_point = hid_map_key(
        key, modifiers_ & (kModifierShift | kModifierCapsLock), keymap_);
    if (ev->key_data->code_point) {
      ev->key_data->modifiers = modifiers_;
    } else {
      ev->key_data->modifiers = 0;
    }

    switch (key) {
      case HID_USAGE_KEY_LEFT_SHIFT:
        modifiers_ |= kModifierLeftShift;
        break;
      case HID_USAGE_KEY_RIGHT_SHIFT:
        modifiers_ |= kModifierRightShift;
        break;
      case HID_USAGE_KEY_LEFT_CTRL:
        modifiers_ |= kModifierLeftControl;
        break;
      case HID_USAGE_KEY_RIGHT_CTRL:
        modifiers_ |= kModifierRightControl;
        break;
      case HID_USAGE_KEY_LEFT_ALT:
        modifiers_ |= kModifierLeftAlt;
        break;
      case HID_USAGE_KEY_RIGHT_ALT:
        modifiers_ |= kModifierRightAlt;
        break;
      case HID_USAGE_KEY_LEFT_GUI:
        modifiers_ |= kModifierLeftSuper;
        break;
      case HID_USAGE_KEY_RIGHT_GUI:
        modifiers_ |= kModifierRightSuper;
        break;
      default:
        break;
    }
    callback(std::move(ev));
  }

  for (KeyUsage key : old_keys) {
    mozart::EventPtr ev = mozart::Event::New();
    ev->action = mozart::EventType::KEY_RELEASED;
    ev->flags = mozart::EventFlags::NONE;
    ev->time_stamp = now;

    ev->key_data = mozart::KeyData::New();
    ev->key_data->hid_usage = key;
    ev->key_data->code_point = hid_map_key(
        key, modifiers_ & (kModifierShift | kModifierCapsLock), keymap_);
    if (ev->key_data->code_point) {
      ev->key_data->modifiers = modifiers_;
    } else {
      ev->key_data->modifiers = 0;
    }

    switch (key) {
      case HID_USAGE_KEY_LEFT_SHIFT:
        modifiers_ &= (~kModifierLeftShift);
        break;
      case HID_USAGE_KEY_RIGHT_SHIFT:
        modifiers_ &= (~kModifierRightShift);
        break;
      case HID_USAGE_KEY_LEFT_CTRL:
        modifiers_ &= (~kModifierLeftControl);
        break;
      case HID_USAGE_KEY_RIGHT_CTRL:
        modifiers_ &= (~kModifierRightControl);
        break;
      case HID_USAGE_KEY_LEFT_ALT:
        modifiers_ &= (~kModifierLeftAlt);
        break;
      case HID_USAGE_KEY_RIGHT_ALT:
        modifiers_ &= (~kModifierRightAlt);
        break;
      case HID_USAGE_KEY_LEFT_GUI:
        modifiers_ &= (~kModifierLeftSuper);
        break;
      case HID_USAGE_KEY_RIGHT_GUI:
        modifiers_ &= (~kModifierRightSuper);
        break;
      case HID_USAGE_KEY_CAPSLOCK:
        if (modifiers_ & kModifierCapsLock) {
          modifiers_ &= (~kModifierCapsLock);
        } else {
          modifiers_ |= kModifierCapsLock;
        }
        break;
      default:
        break;
    }
    callback(std::move(ev));
  }
}

#pragma mark - MouseState

void MouseState::SendEvent(float rel_x,
                           float rel_y,
                           int64_t timestamp,
                           mozart::EventType type,
                           mozart::EventFlags flags,
                           OnEventCallback callback) {
  mozart::EventPtr ev = mozart::Event::New();
  ev->time_stamp = timestamp;
  ev->action = type;
  ev->flags = flags;

  ev->pointer_data = mozart::PointerData::New();
  ev->pointer_data->pointer_id = 0;
  ev->pointer_data->kind = mozart::PointerKind::MOUSE;
  ev->pointer_data->x = rel_x;
  ev->pointer_data->y = rel_y;
  ev->pointer_data->screen_x = rel_x;
  ev->pointer_data->screen_y = rel_y;
  ev->pointer_data->pressure = 0;
  ev->pointer_data->radius_major = 0;
  ev->pointer_data->radius_minor = 0;
  ev->pointer_data->orientation = 0.;
  ev->pointer_data->horizontal_wheel = 0;
  ev->pointer_data->vertical_wheel = 0;
  callback(std::move(ev));
}

void MouseState::Update(const MouseReport& report,
                        const MouseDescriptor& descriptor,
                        mozart::Size display_size,
                        OnEventCallback callback) {
  uint64_t now = InputEventTimestampNow();
  uint8_t pressed = (report.buttons ^ buttons_) & report.buttons;
  uint8_t released = (report.buttons ^ buttons_) & buttons_;

  position_.x =
      std::max(0.0f, std::min(position_.x + report.rel_x,
                              static_cast<float>(display_size.width)));
  position_.y =
      std::max(0.0f, std::min(position_.y - report.rel_y,
                              static_cast<float>(display_size.height)));

  if (!pressed && !released) {
    SendEvent(position_.x, position_.y, now, mozart::EventType::POINTER_MOVE,
              mozart::EventFlags::NONE, callback);
  } else {
    if (pressed) {
      if (pressed & kMouseLeftButtonMask) {
        SendEvent(position_.x, position_.y, now,
                  mozart::EventType::POINTER_DOWN,
                  mozart::EventFlags::LEFT_MOUSE_BUTTON, callback);
      }
      if (pressed & kMouseRightButtonMask) {
        SendEvent(position_.x, position_.y, now,
                  mozart::EventType::POINTER_DOWN,
                  mozart::EventFlags::RIGHT_MOUSE_BUTTON, callback);
      }
      if (pressed & kMouseMiddleButtonMask) {
        SendEvent(position_.x, position_.y, now,
                  mozart::EventType::POINTER_DOWN,
                  mozart::EventFlags::MIDDLE_MOUSE_BUTTON, callback);
      }
    }
    if (released) {
      if (released & kMouseLeftButtonMask) {
        SendEvent(position_.x, position_.y, now, mozart::EventType::POINTER_UP,
                  mozart::EventFlags::LEFT_MOUSE_BUTTON, callback);
      }
      if (released & kMouseRightButtonMask) {
        SendEvent(position_.x, position_.y, now, mozart::EventType::POINTER_UP,
                  mozart::EventFlags::RIGHT_MOUSE_BUTTON, callback);
      }
      if (released & kMouseMiddleButtonMask) {
        SendEvent(position_.x, position_.y, now, mozart::EventType::POINTER_UP,
                  mozart::EventFlags::MIDDLE_MOUSE_BUTTON, callback);
      }
    }
  }
  buttons_ = report.buttons;
}

#pragma mark - StylusState

void StylusState::Update(const StylusReport& report,
                         const StylusDescriptor& descriptor,
                         mozart::Size display_size,
                         OnEventCallback callback) {
  const bool previous_stylus_down = stylus_down_;
  stylus_down_ = report.is_down;

  mozart::EventType action = mozart::EventType::POINTER_DOWN;
  if (stylus_down_) {
    if (previous_stylus_down) {
      action = mozart::EventType::POINTER_MOVE;
    }
  } else {
    if (!previous_stylus_down) {
      return;
    }
    action = mozart::EventType::POINTER_UP;
  }
  int64_t now = InputEventTimestampNow();

  mozart::EventPtr ev = mozart::Event::New();
  ev->action = action;
  ev->flags = mozart::EventFlags::NONE;
  ev->time_stamp = now;
  if (action == mozart::EventType::POINTER_UP) {
    ev->pointer_data = stylus_.Clone();
  } else {
    float x =
        static_cast<float>(display_size.width *
                           (report.x - descriptor.x.range.min)) /
        static_cast<float>(descriptor.x.range.max - descriptor.x.range.min);
    float y =
        static_cast<float>(display_size.height *
                           (report.y - descriptor.y.range.min)) /
        static_cast<float>(descriptor.y.range.max - descriptor.y.range.min);

    ev->pointer_data = mozart::PointerData::New();
    ev->pointer_data->pointer_id = 11;  // Assuming max of 10 fingers
    ev->pointer_data->kind = mozart::PointerKind::TOUCH;
    ev->pointer_data->x = x;
    ev->pointer_data->y = y;
    ev->pointer_data->screen_x = x;
    ev->pointer_data->screen_y = y;
    ev->pointer_data->pressure = report.pressure;
    ev->pointer_data->radius_major = 0.;
    ev->pointer_data->radius_minor = 0.;
    ev->pointer_data->orientation = 0.;
    ev->pointer_data->horizontal_wheel = 0.;
    ev->pointer_data->vertical_wheel = 0.;
    stylus_ = *ev->pointer_data;
  }
  callback(std::move(ev));
}

#pragma mark - TouchscreenState

void TouchscreenState::Update(const TouchReport& report,
                              const TouchscreenDescriptor& descriptor,
                              mozart::Size display_size,
                              OnEventCallback callback) {
  std::vector<mozart::PointerData> old_pointers = pointers_;
  pointers_.clear();

  int64_t now = InputEventTimestampNow();

  for (auto touch : report.touches) {
    auto ev = mozart::Event::New();
    ev->action = mozart::EventType::POINTER_DOWN;
    for (auto it = old_pointers.begin(); it != old_pointers.end(); ++it) {
      if (it->pointer_id == touch.finger_id) {
        ev->action = mozart::EventType::POINTER_MOVE;
        old_pointers.erase(it);
        break;
      }
    }
    ev->flags = mozart::EventFlags::NONE;
    ev->time_stamp = now;

    ev->pointer_data = mozart::PointerData::New();
    ev->pointer_data->pointer_id = touch.finger_id;
    ev->pointer_data->kind = mozart::PointerKind::TOUCH;

    float x =
        static_cast<float>(display_size.width *
                           (touch.x - descriptor.x.range.min)) /
        static_cast<float>(descriptor.x.range.max - descriptor.x.range.min);
    float y =
        static_cast<float>(display_size.height *
                           (touch.y - descriptor.y.range.min)) /
        static_cast<float>(descriptor.y.range.max - descriptor.y.range.min);

    uint32_t width = 2 * touch.width;
    uint32_t height = 2 * touch.height;

    ev->pointer_data->x = x;
    ev->pointer_data->y = y;
    ev->pointer_data->screen_x = x;
    ev->pointer_data->screen_y = y;
    ev->pointer_data->pressure = 0.;
    ev->pointer_data->radius_major = width > height ? width : height;
    ev->pointer_data->radius_minor = width > height ? height : width;
    ev->pointer_data->orientation = 0.;
    ev->pointer_data->horizontal_wheel = 0.;
    ev->pointer_data->vertical_wheel = 0.;
    pointers_.push_back(*ev->pointer_data);
    callback(std::move(ev));
  }

  for (const auto& pointer : old_pointers) {
    auto ev = mozart::Event::New();
    ev->action = mozart::EventType::POINTER_UP;
    ev->flags = mozart::EventFlags::NONE;
    ev->time_stamp = now;
    ev->pointer_data = pointer.Clone();
    callback(std::move(ev));
  }
}

}  // namespace input
}  // namespace mozart
