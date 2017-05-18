// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/composer/session/session.h"

#include "apps/mozart/src/composer/print_op.h"
#include "apps/mozart/src/composer/resources/nodes/entity_node.h"
#include "apps/mozart/src/composer/resources/nodes/node.h"
#include "apps/mozart/src/composer/resources/nodes/shape_node.h"
#include "apps/mozart/src/composer/resources/shapes/circle_shape.h"
#include "apps/mozart/src/composer/util/unwrap.h"

namespace mozart {
namespace composer {

Session::Session(SessionId id,
                 SessionContext* context,
                 ErrorReporter* error_reporter)
    : id_(id),
      context_(context),
      error_reporter_(error_reporter),
      resources_(error_reporter) {
  FTL_DCHECK(context);
  FTL_DCHECK(error_reporter);
}

Session::~Session() {
  FTL_DCHECK(!is_valid_);
}

bool Session::ApplyOp(const mozart2::OpPtr& op) {
  switch (op->which()) {
    case mozart2::Op::Tag::CREATE_RESOURCE:
      return ApplyCreateResourceOp(op->get_create_resource());
    case mozart2::Op::Tag::RELEASE_RESOURCE:
      return ApplyReleaseResourceOp(op->get_release_resource());
    case mozart2::Op::Tag::ADD_CHILD:
      return ApplyAddChildOp(op->get_add_child());
    case mozart2::Op::Tag::ADD_PART:
      return ApplyAddPartOp(op->get_add_part());
    case mozart2::Op::Tag::DETACH:
      return ApplyDetachOp(op->get_detach());
    case mozart2::Op::Tag::DETACH_CHILDREN:
      return ApplyDetachChildrenOp(op->get_detach_children());
    case mozart2::Op::Tag::SET_TRANSFORM:
      return ApplySetTransformOp(op->get_set_transform());
    case mozart2::Op::Tag::SET_SHAPE:
      return ApplySetShapeOp(op->get_set_shape());
    case mozart2::Op::Tag::SET_MATERIAL:
      return ApplySetMaterialOp(op->get_set_material());
    case mozart2::Op::Tag::SET_CLIP:
      return ApplySetClipOp(op->get_set_clip());
    default:
      error_reporter_->ERROR()
          << "composer::Session::ApplyOp(): unimplemented op: " << op;
      return false;
  }

  return true;
}

bool Session::ApplyCreateResourceOp(const mozart2::CreateResourceOpPtr& op) {
  const ResourceId id = op->id;
  if (id == 0) {
    error_reporter_->ERROR()
        << "composer::Session::ApplyCreateResourceOp(): invalid ID: " << op;
    return false;
  }

  switch (op->resource->which()) {
    case mozart2::Resource::Tag::MEMORY:
      return ApplyCreateMemory(id, op->resource->get_memory());
    case mozart2::Resource::Tag::IMAGE:
      return ApplyCreateImage(id, op->resource->get_image());
    case mozart2::Resource::Tag::BUFFER:
      return ApplyCreateBuffer(id, op->resource->get_buffer());
    case mozart2::Resource::Tag::LINK:
      return ApplyCreateLink(id, op->resource->get_link());
    case mozart2::Resource::Tag::RECTANGLE:
      return ApplyCreateRectangle(id, op->resource->get_rectangle());
    case mozart2::Resource::Tag::CIRCLE:
      return ApplyCreateCircle(id, op->resource->get_circle());
    case mozart2::Resource::Tag::MESH:
      return ApplyCreateMesh(id, op->resource->get_mesh());
    case mozart2::Resource::Tag::MATERIAL:
      return ApplyCreateMaterial(id, op->resource->get_material());
    case mozart2::Resource::Tag::CLIP_NODE:
      return ApplyCreateClipNode(id, op->resource->get_clip_node());
    case mozart2::Resource::Tag::ENTITY_NODE:
      return ApplyCreateEntityNode(id, op->resource->get_entity_node());
    case mozart2::Resource::Tag::LINK_NODE:
      return ApplyCreateLinkNode(id, op->resource->get_link_node());
    case mozart2::Resource::Tag::SHAPE_NODE:
      return ApplyCreateShapeNode(id, op->resource->get_shape_node());
    case mozart2::Resource::Tag::TAG_NODE:
      return ApplyCreateTagNode(id, op->resource->get_tag_node());
    default:
      error_reporter_->ERROR()
          << "composer::Session::ApplyCreateResourceOp(): unsupported resource"
          << op;
      return false;
  }
}

bool Session::ApplyReleaseResourceOp(const mozart2::ReleaseResourceOpPtr& op) {
  return resources_.RemoveResource(op->id);
}

bool Session::ApplyAddChildOp(const mozart2::AddChildOpPtr& op) {
  // Find the parent and child nodes.
  auto parent_node = resources_.FindResource<Node>(op->node_id);
  if (!parent_node) {
    error_reporter_->ERROR() << "composer::Session::ApplyAddChildOp(): cannot "
                                "find parent node with ID "
                             << op->node_id;
    return false;
  }
  auto child_node = resources_.FindResource<Node>(op->child_id);
  if (!child_node) {
    error_reporter_->ERROR() << "composer::Session::ApplyAddChildOp(): cannot "
                                "find child node with ID "
                             << op->child_id;
    return false;
  }

  return parent_node->AddChild(std::move(child_node));
}

bool Session::ApplyAddPartOp(const mozart2::AddPartOpPtr& op) {
  // Find the parent and part nodes.
  auto parent_node = resources_.FindResource<Node>(op->node_id);
  if (!parent_node) {
    error_reporter_->ERROR() << "composer::Session::ApplyAddPartOp(): cannot "
                                "find parent node with ID "
                             << op->node_id;
    return false;
  }
  auto part_node = resources_.FindResource<Node>(op->part_id);
  if (!part_node) {
    error_reporter_->ERROR()
        << "composer::Session::ApplyAddPartOp(): cannot find part node with ID "
        << op->part_id;
    return false;
  }

  return parent_node->AddPart(std::move(part_node));
}

bool Session::ApplyDetachOp(const mozart2::DetachOpPtr& op) {
  auto node = resources_.FindResource<Node>(op->node_id);
  if (!node) {
    error_reporter_->ERROR()
        << "composer::Session::ApplyDetachOp(): cannot find node with ID "
        << op->node_id;
    return false;
  }
  return Node::Detach(node);
}

bool Session::ApplyDetachChildrenOp(const mozart2::DetachChildrenOpPtr& op) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyDetachChildrenOp(): unimplemented";
  return false;
}

bool Session::ApplySetTransformOp(const mozart2::SetTransformOpPtr& op) {
  auto node = resources_.FindResource<Node>(op->node_id);
  if (!node) {
    error_reporter_->ERROR() << "composer::Session::ApplySetTransformOp(): "
                                "could not find Node with ID "
                             << op->node_id;
    return false;
  }
  return node->SetTransform(Unwrap(op->transform));
}

bool Session::ApplySetShapeOp(const mozart2::SetShapeOpPtr& op) {
  auto node = resources_.FindResource<ShapeNode>(op->node_id);
  if (!node) {
    error_reporter_->ERROR() << "composer::Session::ApplySetMaterialOp(): "
                                "could not find ShapeNode with ID "
                             << op->node_id;
    return false;
  }
  auto shape = resources_.FindResource<Shape>(op->shape_id);
  if (!shape) {
    error_reporter_->ERROR() << "composer::Session::ApplySetMaterialOp(): "
                                "could not find Shape with ID "
                             << op->shape_id;
    return false;
  }
  node->SetShape(std::move(shape));
  return true;
}

bool Session::ApplySetMaterialOp(const mozart2::SetMaterialOpPtr& op) {
  auto node = resources_.FindResource<ShapeNode>(op->node_id);
  if (!node) {
    error_reporter_->ERROR() << "composer::Session::ApplySetMaterialOp(): "
                                "could not find ShapeNode with ID "
                             << op->node_id;
    return false;
  }
  auto material = resources_.FindResource<Material>(op->material_id);
  if (!material) {
    error_reporter_->ERROR() << "composer::Session::ApplySetMaterialOp(): "
                                "could not find Material with ID "
                             << op->material_id;
    return false;
  }
  node->SetMaterial(std::move(material));
  return true;
}

bool Session::ApplySetClipOp(const mozart2::SetClipOpPtr& op) {
  error_reporter_->ERROR()
      << "composer::Session::ApplySetClipOp(): unimplemented";
  return false;
}

bool Session::ApplyCreateMemory(ResourceId id, const mozart2::MemoryPtr& args) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyCreateMemory(): unimplemented";
  return false;
}

bool Session::ApplyCreateImage(ResourceId id, const mozart2::ImagePtr& args) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyCreateImage(): unimplemented";
  return false;
}

bool Session::ApplyCreateBuffer(ResourceId id, const mozart2::BufferPtr& args) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyCreateBuffer(): unimplemented";
  return false;
}

bool Session::ApplyCreateLink(ResourceId id, const mozart2::LinkPtr& args) {
  auto link = context_->CreateLink(this, args);
  return link ? resources_.AddResource(id, std::move(link)) : false;
}

bool Session::ApplyCreateRectangle(ResourceId id,
                                   const mozart2::RectanglePtr& args) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyCreateRectangle(): unimplemented";
  return false;
}

bool Session::ApplyCreateCircle(ResourceId id, const mozart2::CirclePtr& args) {
  auto tag = args->radius->which();
  if (tag == mozart2::Value::Tag::VECTOR1) {
    float initial_radius = args->radius->get_vector1();
    auto circle = CreateCircle(id, initial_radius);
    return circle ? resources_.AddResource(id, std::move(circle)) : false;
  } else if (tag == mozart2::Value::Tag::VARIABLE_ID) {
    error_reporter_->ERROR() << "composer::Session::ApplyCreateCircle(): "
                                "unimplemented: variable radius";
    return false;
  } else {
    error_reporter_->ERROR() << "composer::Session::ApplyCreateCircle(): "
                                "radius must be a float or a variable of type "
                                "float";
    return false;
  }
}

bool Session::ApplyCreateMesh(ResourceId id, const mozart2::MeshPtr& args) {
  error_reporter_->ERROR()
      << "composer::Session::ApplyCreateMesh(): unimplemented";
  return false;
}

bool Session::ApplyCreateMaterial(ResourceId id,
                                  const mozart2::MaterialPtr& args) {
  if (args->texture_id != 0) {
    error_reporter_->ERROR() << "composer::Session::ApplyCreateMaterial(): "
                                "unimplemented: texture support";
    return false;
  }

  auto& color = args->color;
  auto material = CreateMaterial(id, static_cast<float>(color->red) / 255.f,
                                 static_cast<float>(color->green) / 255.f,
                                 static_cast<float>(color->blue) / 255.f,
                                 static_cast<float>(color->alpha) / 255.f);
  return material ? resources_.AddResource(id, std::move(material)) : false;
}

bool Session::ApplyCreateClipNode(ResourceId id,
                                  const mozart2::ClipNodePtr& args) {
  auto node = CreateClipNode(id, args);
  return node ? resources_.AddResource(id, std::move(node)) : false;
}

bool Session::ApplyCreateEntityNode(ResourceId id,
                                    const mozart2::EntityNodePtr& args) {
  auto node = CreateEntityNode(id, args);
  return node ? resources_.AddResource(id, std::move(node)) : false;
}

bool Session::ApplyCreateLinkNode(ResourceId id,
                                  const mozart2::LinkNodePtr& args) {
  auto node = CreateLinkNode(id, args);
  return node ? resources_.AddResource(id, std::move(node)) : false;
}

bool Session::ApplyCreateShapeNode(ResourceId id,
                                   const mozart2::ShapeNodePtr& args) {
  auto node = CreateShapeNode(id, args);
  return node ? resources_.AddResource(id, std::move(node)) : false;
}

bool Session::ApplyCreateTagNode(ResourceId id,
                                 const mozart2::TagNodePtr& args) {
  auto node = CreateTagNode(id, args);
  return node ? resources_.AddResource(id, std::move(node)) : false;
}

ResourcePtr Session::CreateClipNode(ResourceId id,
                                    const mozart2::ClipNodePtr& args) {
  error_reporter_->ERROR() << "composer::Session::CreateClipNode(): "
                              "unimplemented.";
  return ResourcePtr();
}

ResourcePtr Session::CreateEntityNode(ResourceId id,
                                      const mozart2::EntityNodePtr& args) {
  return ftl::MakeRefCounted<EntityNode>(this);
}

ResourcePtr Session::CreateLinkNode(ResourceId id,
                                    const mozart2::LinkNodePtr& args) {
  error_reporter_->ERROR() << "composer::Session::CreateLinkNode(): "
                              "unimplemented.";
  return ResourcePtr();
}

ResourcePtr Session::CreateShapeNode(ResourceId id,
                                     const mozart2::ShapeNodePtr& args) {
  return ftl::MakeRefCounted<ShapeNode>(this);
}

ResourcePtr Session::CreateTagNode(ResourceId id,
                                   const mozart2::TagNodePtr& args) {
  error_reporter_->ERROR() << "composer::Session::CreateTagNode(): "
                              "unimplemented.";
  return ResourcePtr();
}

ResourcePtr Session::CreateCircle(ResourceId id, float initial_radius) {
  return ftl::MakeRefCounted<CircleShape>(this, initial_radius);
}

ResourcePtr Session::CreateMaterial(ResourceId id,
                                    float red,
                                    float green,
                                    float blue,
                                    float alpha) {
  return ftl::MakeRefCounted<Material>(this, red, green, blue, alpha);
}

void Session::TearDown() {
  if (!is_valid_) {
    // TearDown already called.
    return;
  }
  is_valid_ = false;
  error_reporter_ = nullptr;
  resources_.Clear();

  context_->OnSessionTearDown(this);
  FTL_DCHECK(resource_count_ == 0);
}

ErrorReporter* Session::error_reporter() const {
  return error_reporter_ ? error_reporter_ : ErrorReporter::Default();
}

}  // namespace composer
}  // namespace mozart
