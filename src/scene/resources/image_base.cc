// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/resources/image_base.h"

namespace mozart {
namespace scene {

const ResourceTypeInfo ImageBase::kTypeInfo = {ResourceType::kImageBase,
                                               "ImageBase"};

ImageBase::ImageBase(Session* session, const ResourceTypeInfo& type_info)
    : Resource(session, type_info) {
  FTL_DCHECK(type_info.IsKindOf(ImageBase::kTypeInfo));
}

}  // namespace scene
}  // namespace mozart
