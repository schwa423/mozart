// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "apps/mozart/src/sketchy/buffer.h"
#include "apps/mozart/src/sketchy/resources/resource.h"

namespace sketchy_service {

class StrokeGroup;
using StrokeGroupPtr = ftl::RefPtr<StrokeGroup>;

class StrokeGroup final : public Resource {
 public:
  static const ResourceTypeInfo kTypeInfo;
  const ResourceTypeInfo& type_info() const override { return kTypeInfo; }

  StrokeGroup(mozart::client::Session* session,
              escher::BufferFactory* buffer_factory);

  // TODO(MZ-269): No Stroke class exists.
  // bool AddStroke(StrokePtr stroke);
  // bool RemoveStroke(StrokePtr stroke);

  const mozart::client::ShapeNode& shape_node() const { return shape_node_; }

 private:
  mozart::client::ShapeNode shape_node_;
  mozart::client::Material material_;
  mozart::client::Mesh mesh_;

  // TODO: more sophisticated buffer management.
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> index_buffer_;

  FTL_DISALLOW_COPY_AND_ASSIGN(StrokeGroup);
};

}  // namespace sketchy_service
