// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/composer/resources/nodes/node.h"

#include "apps/mozart/src/composer/resources/material.h"
#include "apps/mozart/src/composer/resources/shapes/shape.h"

namespace mozart {
namespace composer {

class ShapeNode final : public Node {
 public:
  static const ResourceTypeInfo kTypeInfo;

  ShapeNode(Session* session);

  void SetMaterial(MaterialPtr material);
  void SetShape(ShapePtr shape);

  const MaterialPtr& material() const { return material_; }
  const ShapePtr& shape() const { return shape_; }

 private:
  MaterialPtr material_;
  ShapePtr shape_;
};

}  // namespace composer
}  // namespace mozart
