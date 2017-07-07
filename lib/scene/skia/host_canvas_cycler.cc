// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/lib/scene/skia/host_canvas_cycler.h"

#include "lib/ftl/logging.h"

namespace mozart {
namespace skia {

HostCanvasCycler::HostCanvasCycler(mozart::client::Session* session)
    : EntityNode(session),
      content_node_(session),
      content_material_(session),
      surface_pool_(session, kNumBuffers) {
  content_node_.SetMaterial(content_material_);
  AddChild(content_node_);
}

HostCanvasCycler::~HostCanvasCycler() = default;

SkCanvas* HostCanvasCycler::AcquireCanvas(uint32_t width, uint32_t height) {
  FTL_DCHECK(!acquired_surface_);

  // Update the surface pool and content shape.
  mozart2::ImageInfo image_info;
  image_info.width = width;
  image_info.height = height;
  image_info.stride = width * 4u;
  image_info.pixel_format = mozart2::ImageInfo::PixelFormat::BGRA_8;
  image_info.color_space = mozart2::ImageInfo::ColorSpace::SRGB;
  image_info.tiling = mozart2::ImageInfo::Tiling::LINEAR;
  reconfigured_ = surface_pool_.Configure(&image_info);

  // Acquire the surface.
  acquired_surface_ = surface_pool_.GetSkSurface(surface_index_);
  FTL_DCHECK(acquired_surface_);
  acquired_surface_->getCanvas()->save();
  return acquired_surface_->getCanvas();
}

void HostCanvasCycler::ReleaseAndSwapCanvas() {
  FTL_DCHECK(acquired_surface_);

  acquired_surface_->getCanvas()->restoreToCount(1);
  acquired_surface_->flush();
  acquired_surface_.reset();

  const mozart::client::HostImage* image =
      surface_pool_.GetImage(surface_index_);
  FTL_DCHECK(image);
  content_material_.SetTexture(*image);

  if (reconfigured_) {
    mozart::client::Rectangle content_rect(content_node_.session(),
                                           surface_pool_.image_info()->width,
                                           surface_pool_.image_info()->height);
    content_node_.SetShape(content_rect);
    reconfigured_ = false;
  }

  // TODO(MZ-145): Define an |InvalidateOp| on |Image| instead.
  surface_pool_.DiscardImage(surface_index_);
  surface_index_ = (surface_index_ + 1) % kNumBuffers;
}

}  // namespace skia
}  // namespace mozart
