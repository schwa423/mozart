// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vulkan/vulkan.hpp>

#include "apps/mozart/src/scene_manager/resources/memory.h"
#include "apps/mozart/src/scene_manager/util/error_reporter.h"
#include "escher/vk/gpu_mem.h"
#include "lib/mtl/vmo/shared_vmo.h"

namespace mozart {
namespace scene {

class HostMemory;
using HostMemoryPtr = ftl::RefPtr<HostMemory>;

// Wraps a CPU host memory-backed VMO.
class HostMemory : public Memory {
 public:
  static const ResourceTypeInfo kTypeInfo;

  // Constructor for host memory.
  HostMemory(Session* session, ResourceId id, mx::vmo vmo, uint64_t vmo_size);

  // Helper method for creating HostMemory object from a mozart2::Memory.
  // Create a HostMemory resource object from a CPU host memory-backed VMO.
  //
  // Returns the created HostMemory object or nullptr if there was an error.
  static HostMemoryPtr New(Session* session,
                           ResourceId id,
                           vk::Device device,
                           mx::vmo vmo,
                           ErrorReporter* error_reporter);

  // Helper method that calls the above method with the VMO from |args|. Also
  // checks the memory type in debug mode.
  static HostMemoryPtr New(Session* session,
                           ResourceId id,
                           vk::Device device,
                           const mozart2::MemoryPtr& args,
                           ErrorReporter* error_reporter);

  void Accept(class ResourceVisitor* visitor) override;

  void* memory_base() { return shared_vmo_->Map(); }
  uint64_t size() const { return size_; }

 private:
  ftl::RefPtr<mtl::SharedVmo> shared_vmo_;
  uint64_t size_;
};

}  // namespace scene
}  // namespace mozart
