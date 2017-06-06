// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/scene/resources/resource_visitor.h"

#include "apps/mozart/src/scene/resources/gpu_memory.h"
#include "apps/mozart/src/scene/resources/host_memory.h"
#include "apps/mozart/src/scene/resources/image.h"
#include "apps/mozart/src/scene/resources/link.h"
#include "apps/mozart/src/scene/resources/material.h"
#include "apps/mozart/src/scene/resources/nodes/entity_node.h"
#include "apps/mozart/src/scene/resources/nodes/node.h"
#include "apps/mozart/src/scene/resources/nodes/shape_node.h"
#include "apps/mozart/src/scene/resources/shapes/circle_shape.h"
#include "apps/mozart/src/scene/resources/shapes/shape.h"

namespace mozart {
namespace scene {

void GpuMemory::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void HostMemory::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void Image::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void EntityNode::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void Node::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void ShapeNode::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void CircleShape::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void Shape::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void Link::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

void Material::Accept(ResourceVisitor* visitor) {
  visitor->Visit(this);
}

}  // namespace scene
}  // namespace mozart
