// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/resources/host_memory.h"

namespace mozart {
namespace scene {

const ResourceTypeInfo HostMemory::kTypeInfo = {
    ResourceType::kMemory | ResourceType::kHostMemory, "HostMemory"};

HostMemory::HostMemory(Session* session, mx::vmo vmo, vk::DeviceSize vmo_size)
    : Memory(session, HostMemory::kTypeInfo),
      shared_vmo_(std::make_unique<mtl::SharedVmo>(std::move(vmo),
                                                   MX_VM_FLAG_PERM_READ)) {}

HostMemoryPtr HostMemory::New(Session* session,
                              vk::Device device,
                              const mozart2::MemoryPtr& args,
                              ErrorReporter* error_reporter) {
  size_t vmo_size;
  args->vmo.get_size(&vmo_size);
  return ftl::MakeRefCounted<HostMemory>(session, std::move(args->vmo),
                                         vmo_size);
}

}  // namespace scene
}  // namespace mozart
