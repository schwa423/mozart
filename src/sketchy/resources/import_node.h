// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/sketchy/resources/resource.h"
#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "escher/base/typed_reffable.h"
#include "lib/ftl/memory/ref_counted.h"

namespace sketchy_service {

// Wrapper of mozart::client::ImportNode. To import a node, client should
// export it as token, and this class takes that token, so that it functions
// as if it were the exported node.
class ImportNode final : public Resource {
 public:
  static const ResourceTypeInfo kTypeInfo;
  const ResourceTypeInfo& type_info() const override { return kTypeInfo; }

  ImportNode(mozart::client::Session* session, mx::eventpair token);
  void AddChild(const mozart::client::Node& child);

  // Resource ID shared with scene manager.
  uint32_t id() { return node_.id(); }

 private:
  mozart::client::ImportNode node_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ImportNode);
};

}  // namespace sketchy_service
