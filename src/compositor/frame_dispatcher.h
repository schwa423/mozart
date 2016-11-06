// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_COMPOSITOR_FRAME_DISPATCHER_H_
#define APPS_MOZART_SRC_COMPOSITOR_FRAME_DISPATCHER_H_

#include <functional>
#include <vector>

#include "apps/mozart/services/composition/scheduling.fidl.h"
#include "lib/ftl/macros.h"

namespace compositor {

using FrameCallback = std::function<void(mozart::FrameInfoPtr)>;

// Maintains a list of pending frame callbacks to be dispatched.
class FrameDispatcher {
 public:
  FrameDispatcher();
  ~FrameDispatcher();

  // Adds a callback, returns true if it was the first pending callback.
  bool AddCallback(const FrameCallback& callback);

  // Dispatches all pending callbacks then clears the list.
  void DispatchCallbacks(const mozart::FrameInfo& frame_info);

 private:
  std::vector<FrameCallback> pending_callbacks_;

  FTL_DISALLOW_COPY_AND_ASSIGN(FrameDispatcher);
};

}  // namespace compositor

#endif  // APPS_MOZART_SRC_COMPOSITOR_FRAME_DISPATCHER_H_
