// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <unordered_map>

#include "apps/mozart/src/sketchy/resources/resource_map.h"
#include "apps/mozart/src/sketchy/resources/types.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "apps/mozart/services/fun/sketchy/canvas.fidl.h"
#include "escher/escher.h"

namespace sketchy_service {

class CanvasImpl final : public sketchy::Canvas {
 public:
  CanvasImpl(mozart::client::Session* session, escher::Escher* escher);

  // |sketchy::Canvas|
  void Init(::fidl::InterfaceHandle<sketchy::CanvasListener> listener) override;
  void Enqueue(::fidl::Array<sketchy::OpPtr> ops) override;
  void Present(
      uint64_t presentation_time, const PresentCallback& callback) override;

 private:
  bool ApplyOp(const sketchy::OpPtr& op);

  bool ApplyCreateResourceOp(const sketchy::CreateResourceOpPtr& op);
  bool ApplyReleaseResourceOp(const sketchy::ReleaseResourceOpPtr& op);
  bool CreateStroke(ResourceId id, const sketchy::StrokePtr& stroke);
  bool CreateStrokeGroup(
      ResourceId id, const sketchy::StrokeGroupPtr& stroke_group);

  bool ApplyAddStrokeOp(const sketchy::AddStrokeOpPtr& op);
  bool ApplyRemoveStrokeOp(const sketchy::RemoveStrokeOpPtr& op);

  bool ApplyScenicImportResourceOp(
      const mozart2::ImportResourceOpPtr& import_resource);

  // Imports an exported ScenicNode that can be used as an
  // attachment point for a StrokeGroup.
  //
  // |id| ID that can be used by the Canvas client to refer to
  //     the imported node.
  // |token| Token that the Sketchy service will pass along
  //     to the SceneManager to import the node.
  bool ScenicImportNode(ResourceId id, mx::eventpair token);

  bool ApplyScenicAddChildOp(const mozart2::AddChildOpPtr& add_child);

  mozart::client::Session* session_;
  ::fidl::Array<sketchy::OpPtr> ops_;
  ResourceMap resource_map_;
};

}  // namespace sketchy_service
