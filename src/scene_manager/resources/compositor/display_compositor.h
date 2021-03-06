// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene_manager/resources/compositor/compositor.h"

namespace scene_manager {

class Display;
class DisplaySwapchain;

// DisplayCompositor is a Compositor that renders directly to the display.
class DisplayCompositor : public Compositor {
 public:
  static const ResourceTypeInfo kTypeInfo;

  DisplayCompositor(Session* session,
                    mozart::ResourceId id,
                    Display* display,
                    std::unique_ptr<DisplaySwapchain> swapchain);

  ~DisplayCompositor() override;

  // | Resource |
  void Accept(class ResourceVisitor* visitor) override;

 private:
  Display* const display_;

  FTL_DISALLOW_COPY_AND_ASSIGN(DisplayCompositor);
};

}  // namespace scene_manager
