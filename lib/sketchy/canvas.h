// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "application/lib/app/application_context.h"
#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "apps/mozart/lib/sketchy/resources.h"
#include "apps/mozart/services/fun/sketchy/canvas.fidl.h"

namespace sketchy_lib {

// Convenient C++ wrapper for sketchy::Canvas service.
class Canvas final {
 public:
  Canvas(app::ApplicationContext* context);
  Canvas(sketchy::CanvasPtr canvas);
  void Present(uint64_t time);

 private:
  friend class Resource;
  ResourceId AllocateResourceId();

  sketchy::CanvasPtr canvas_;
  fidl::Array<sketchy::OpPtr> ops_;
  ResourceId next_resource_id_;
};

}  // namespace sketchy_lib
