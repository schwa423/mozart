// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene_manager/resources/memory.h"

namespace scene_manager {

const ResourceTypeInfo Memory::kTypeInfo = {ResourceType::kMemory, "Memory"};

Memory::Memory(Session* session,
               mozart::ResourceId id,
               const ResourceTypeInfo& type_info)
    : Resource(session, id, type_info) {}

}  // namespace scene_manager
