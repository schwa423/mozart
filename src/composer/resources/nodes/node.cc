// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/composer/resources/nodes/node.h"

#include "apps/mozart/src/composer/util/error_reporter.h"
#include "lib/escher/escher/geometry/types.h"

namespace mozart {
namespace composer {

namespace {

constexpr ResourceTypeFlags kHasChildren =
    ResourceType::kEntityNode | ResourceType::kLink | ResourceType::kTagNode;
constexpr ResourceTypeFlags kHasParts =
    ResourceType::kEntityNode | ResourceType::kClipNode;

constexpr ResourceTypeFlags kHasTransform =
    ResourceType::kClipNode | ResourceType::kEntityNode |
    ResourceType::kLinkNode | ResourceType::kShapeNode;

}  // anonymous namespace

const ResourceTypeInfo Node::kTypeInfo = {ResourceType::kNode, "Node"};

Node::Node(Session* session,
           ResourceId node_id,
           const ResourceTypeInfo& type_info)
    : Resource(session, type_info), resource_id_(node_id) {
  FTL_DCHECK(type_info.IsKindOf(Node::kTypeInfo));
}

Node::~Node() = default;

bool Node::AddChild(NodePtr child_node) {
  if (!(type_flags() & kHasChildren)) {
    error_reporter()->ERROR() << "composer::Node::AddChild(): node of type "
                              << type_name() << " cannot have children.";
    return false;
  }

  // Remove child from current parent, if necessary.
  if (auto parent = child_node->parent_) {
    if (this == parent && !child_node->is_part_) {
      // Node is already our child.
      return true;
    }
    // Remove child from parent.
    Detach(child_node);
  }

  // Add child to its new parent (i.e. us).
  child_node->is_part_ = false;
  child_node->parent_ = this;
  child_node->InvalidateGlobalTransform();

  auto insert_result = children_.insert(std::move(child_node));
  FTL_DCHECK(insert_result.second);

  return true;
}

bool Node::AddPart(NodePtr part_node) {
  if (!(type_flags() & kHasParts)) {
    error_reporter()->ERROR() << "composer::Node::AddPart(): node of type "
                              << type_name() << " cannot have parts.";
    return false;
  }

  // Remove part from current parent, if necessary.
  if (auto parent = part_node->parent_) {
    if (this == parent && part_node->is_part_) {
      // Node is already our child.
      return true;
    }
    Detach(part_node);
  }

  // Add part to its new parent (i.e. us).
  part_node->is_part_ = true;
  part_node->parent_ = this;
  part_node->InvalidateGlobalTransform();

  auto insert_result = children_.insert(std::move(part_node));
  FTL_DCHECK(insert_result.second);

  return true;
}

bool Node::Detach(const NodePtr& node_to_detach_from_parent) {
  FTL_DCHECK(node_to_detach_from_parent);
  if (node_to_detach_from_parent->type_flags() & ResourceType::kLink) {
    node_to_detach_from_parent->error_reporter()->ERROR()
        << "A Link cannot be detached.";
    return false;
  }
  if (auto parent = node_to_detach_from_parent->parent_) {
    auto& container = node_to_detach_from_parent->is_part_ ? parent->parts_
                                                           : parent->children_;
    size_t removed_count = container.erase(node_to_detach_from_parent);
    FTL_DCHECK(removed_count == 1);  // verify parent-child invariant
    node_to_detach_from_parent->parent_ = nullptr;
    node_to_detach_from_parent->InvalidateGlobalTransform();
  }
  return true;
}

bool Node::SetTransform(const escher::Transform& transform) {
  if (!(type_flags() & kHasTransform)) {
    error_reporter()->ERROR() << "composer::Node::SetTransform(): node of type "
                              << type_name() << " cannot have transform set.";
    return false;
  }
  transform_ = transform;
  InvalidateGlobalTransform();
  return true;
}

void Node::InvalidateGlobalTransform() {
  if (!global_transform_dirty_) {
    global_transform_dirty_ = true;
    for (auto& node : parts_) {
      node->InvalidateGlobalTransform();
    }
    for (auto& node : children_) {
      node->InvalidateGlobalTransform();
    }
  }
}

void Node::ComputeGlobalTransform() const {
  if (parent_) {
    global_transform_ =
        parent_->GetGlobalTransform() * static_cast<escher::mat4>(transform_);
  } else {
    global_transform_ = static_cast<escher::mat4>(transform_);
  }
}

escher::vec2 Node::ConvertPointFromNode(const escher::vec2& point,
                                        const Node& node) const {
  auto inverted = glm::inverse(GetGlobalTransform());
  auto adjusted =
      glm::vec4{point.x, point.y, 0.0f, 1.0f} * node.GetGlobalTransform();
  auto result = adjusted * inverted;
  return {result.x, result.y};
}

bool Node::ContainsPoint(const escher::vec2& point) const {
  bool inside = false;
  ApplyOnDescendants([&inside, &point](const Node& descendant) -> bool {
    if (descendant.ContainsPoint(point)) {
      inside = true;
      // At least one of our descendants has accepted the hit test. We no longer
      // need to traverse to find the node. Return false and stop the iteration.
      return false;
    }
    return true;
  });
  return inside;
}

void Node::ApplyOnDescendants(std::function<bool(const Node&)> applier) const {
  if (!applier) {
    return;
  }

  for (const auto& node : children_) {
    if (!applier(*node)) {
      return;
    }
  }

  for (const auto& node : parts_) {
    if (!applier(*node)) {
      return;
    }
  }
}

}  // namespace composer
}  // namespace mozart
