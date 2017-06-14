// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene/resources/nodes/scene.h"
#include "apps/mozart/src/scene/resources/proxy_resource.h"
#include "apps/mozart/src/scene/resources/resource_linker.h"
#include "escher/escher.h"
#include "escher/impl/gpu_uploader.h"
#include "escher/resources/resource_recycler.h"
#include "lib/escher/escher/renderer/simple_image_factory.h"
#include "lib/escher/escher/shape/rounded_rect_factory.h"

namespace mozart {
namespace scene {

class Session;

// Interface that describes the ways that a |Session| communicates with its
// environment.
class SessionContext final {
 public:
  SessionContext();

  SessionContext(escher::Escher* escher);

  ~SessionContext();

  ResourceLinker& GetResourceLinker();

  // Register a resource so that it can be imported into a different session
  // via ImportResourceOp.  Return true if successful, and false if the params
  // are invalid.
  bool ExportResource(ResourcePtr resource, mx::eventpair endpoint);

  // Return a new resource in the importing session that acts as a proxy for
  // a resource that was exported by another session.  Return nullptr if the
  // params are invalid.
  void ImportResource(ProxyResourcePtr proxy,
                      mozart2::ImportSpec spec,
                      const mx::eventpair& endpoint);

  ScenePtr CreateScene(Session* session,
                       ResourceId node_id,
                       const mozart2::ScenePtr& args);

  void OnSessionTearDown(Session* session);

  const std::vector<ScenePtr>& scenes() const { return scenes_; }

  vk::Device vk_device() { return vk_device_; }

  escher::ResourceRecycler* escher_resource_recycler() {
    return resource_recycler_;
  }

  escher::ImageFactory* escher_image_factory() { return image_factory_.get(); }

  escher::impl::GpuUploader* escher_gpu_uploader() { return gpu_uploader_; }

  escher::RoundedRectFactory* escher_rounded_rect_factory() {
    return rounded_rect_factory_.get();
  }

 private:
  ResourceLinker resource_linker_;
  vk::Device vk_device_;
  escher::ResourceRecycler* resource_recycler_;
  std::unique_ptr<escher::SimpleImageFactory> image_factory_;
  escher::impl::GpuUploader* gpu_uploader_;
  std::unique_ptr<escher::RoundedRectFactory> rounded_rect_factory_;
  std::vector<ScenePtr> scenes_;

  void OnImportResolvedForResource(
      ProxyResource* proxy,
      ResourcePtr actual,
      ResourceLinker::ResolutionResult resolution_result);

  FTL_DISALLOW_COPY_AND_ASSIGN(SessionContext);
};

}  // namespace scene
}  // namespace mozart
