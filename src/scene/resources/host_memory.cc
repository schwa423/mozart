// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/resources/host_memory.h"

//#include "apps/mozart/services/images2/memory_type.fidl.h"

namespace mozart {
namespace scene {

const ResourceTypeInfo HostMemory::kTypeInfo = {
    ResourceType::kMemory | ResourceType::kHostMemory, "HostMemory"};

HostMemory::HostMemory(Session* session, mx::vmo vmo, uint64_t vmo_size)
    : Memory(session, HostMemory::kTypeInfo),
      shared_vmo_(ftl::MakeRefCounted<mtl::SharedVmo>(std::move(vmo),
                                                      MX_VM_FLAG_PERM_READ)),
      size_(vmo_size) {}

HostMemoryPtr HostMemory::New(Session* session,
                              vk::Device device,
                              const mozart2::MemoryPtr& args,
                              ErrorReporter* error_reporter) {
  if (args->memory_type != mozart2::MemoryType::HOST_MEMORY) {
    error_reporter->ERROR() << "scene::HostMemory::New(): "
                               "Memory must be of type HOST_MEMORY.";
    return nullptr;
  }
  return New(session, device, std::move(args->vmo), error_reporter);
}

HostMemoryPtr HostMemory::New(Session* session,
                              vk::Device device,
                              mx::vmo vmo,
                              ErrorReporter* error_reporter) {
  uint64_t vmo_size;
  vmo.get_size(&vmo_size);
  return ftl::MakeRefCounted<HostMemory>(session, std::move(vmo), vmo_size);
}

}  // namespace scene
}  // namespace mozart
