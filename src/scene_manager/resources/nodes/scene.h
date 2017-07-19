// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene_manager/resources/nodes/node.h"
#include "lib/ftl/macros.h"

namespace scene_manager {

class Scene;
using ScenePtr = ftl::RefPtr<Scene>;

class Scene final : public Node {
 public:
  static const ResourceTypeInfo kTypeInfo;

  Scene(Session* session, mozart::ResourceId node_id);

  // |Resource|.
  void Accept(class ResourceVisitor* visitor) override;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(Scene);
};

}  // namespace scene_manager
