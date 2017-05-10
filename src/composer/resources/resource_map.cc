// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/composer/resources/resource_map.h"

namespace mozart {
namespace composer {

ResourceMap::ResourceMap(ErrorReporter* error_reporter)
    : error_reporter_(error_reporter) {}

ResourceMap::~ResourceMap() {}

void ResourceMap::Clear() {
  resources_.clear();
}

bool ResourceMap::AddResource(ResourceId id, ResourcePtr resource) {
  FTL_DCHECK(resource);

  auto result = resources_.insert(std::make_pair(id, std::move(resource)));
  if (!result.second) {
    error_reporter_->ERROR()
        << "composer::ResourceMap::AddResource(): resource with ID " << id
        << " already exists.";
    return false;
  }
  return true;
}

bool ResourceMap::RemoveResource(ResourceId id) {
  size_t erased_count = resources_.erase(id);
  if (erased_count == 0) {
    error_reporter_->ERROR()
        << "composer::ResourceMap::RemoveResource(): no resource with ID "
        << id;
    return false;
  }
  return true;
}

}  // namespace composer
}  // namespace mozart
