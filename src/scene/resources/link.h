// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene/resources/nodes/node.h"

namespace mozart {
namespace composer {

class Link final : public Node {
 public:
  static const ResourceTypeInfo kTypeInfo;

  Link(Session* session, ResourceId node_id);

  void Accept(class ResourceVisitor* visitor) override;
};

typedef ftl::RefPtr<Link> LinkPtr;

}  // namespace composer
}  // namespace mozart
