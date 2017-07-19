// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene_manager/tests/util.h"

#include <mx/vmo.h>

#include "lib/ftl/synchronization/waitable_event.h"

namespace mozart {
namespace scene {
namespace test {

bool IsEventSignalled(const mx::event& fence, mx_signals_t signal) {
  mx_signals_t pending = 0u;
  fence.wait_one(signal, 0, &pending);
  return (pending & signal) != 0u;
}

mx::event CopyEvent(const mx::event& event) {
  mx::event event_copy;
  if (event.duplicate(MX_RIGHT_SAME_RIGHTS, &event_copy) != MX_OK)
    FTL_LOG(ERROR) << "Copying mx::event failed.";
  return event_copy;
}

mx::vmo CopyVmo(const mx::vmo& vmo) {
  mx::vmo vmo_copy;
  if (vmo.duplicate(MX_RIGHT_SAME_RIGHTS, &vmo_copy) != MX_OK)
    FTL_LOG(ERROR) << "Copying mx::vmo failed.";
  return vmo_copy;
}

ftl::RefPtr<mtl::SharedVmo> CreateSharedVmo(size_t size) {
  mx::vmo vmo;
  mx_status_t status = mx::vmo::create(size, 0u, &vmo);
  if (status != MX_OK) {
    FTL_LOG(ERROR) << "Failed to create vmo: status=" << status
                   << ", size=" << size;
    return nullptr;
  }

  // Optimization: We will be writing to every page of the buffer, so
  // allocate physical memory for it eagerly.
  status = vmo.op_range(MX_VMO_OP_COMMIT, 0u, size, nullptr, 0u);
  if (status != MX_OK) {
    FTL_LOG(ERROR) << "Failed to commit all pages of vmo: status=" << status
                   << ", size=" << size;
    return nullptr;
  }

  uint32_t map_flags = MX_VM_FLAG_PERM_READ | MX_VM_FLAG_PERM_WRITE;
  return ftl::MakeRefCounted<mtl::SharedVmo>(std::move(vmo), map_flags);
}

}  // namespace test
}  // namespace scene
}  // namespace mozart
