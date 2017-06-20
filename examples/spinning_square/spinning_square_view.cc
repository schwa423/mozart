// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/examples/spinning_square/spinning_square_view.h"

#include <math.h>

#include <algorithm>

#include "lib/ftl/logging.h"

namespace examples {
namespace {
constexpr float kBackgroundElevation = 0.f;
constexpr float kSquareElevation = 8.f;

constexpr float kSpeed = 0.25f;
constexpr float kSecondsPerNanosecond = .000'000'001f;
}  // namespace

SpinningSquareView::SpinningSquareView(
    mozart::ViewManagerPtr view_manager,
    fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request)
    : BaseView(std::move(view_manager),
               std::move(view_owner_request),
               "Spinning Square"),
      background_node_(session()),
      square_node_(session()) {
  mozart::client::Material background_material(session());
  background_material.SetColor(0x67, 0x3a, 0xb7, 0xff);  // Deep Purple 500
  background_node_.SetMaterial(background_material);
  parent_node().AddChild(background_node_);

  mozart::client::Material square_material(session());
  square_material.SetColor(0xf5, 0x00, 0x57, 0xff);  // Pink A400
  square_node_.SetMaterial(square_material);
  parent_node().AddChild(square_node_);
}

SpinningSquareView::~SpinningSquareView() {}

void SpinningSquareView::OnPropertiesChanged(
    mozart::ViewPropertiesPtr old_properties) {
  InvalidateScene();
}

void SpinningSquareView::OnSceneInvalidated(
    mozart2::PresentationInfoPtr presentation_info) {
  if (!has_size())
    return;

  uint64_t presentation_time = presentation_info->presentation_time;
  if (!start_time_)
    start_time_ = presentation_time;

  const float center_x = size().width * .5f;
  const float center_y = size().height * .5f;
  const float square_size = std::min(size().width, size().height) * .6f;
  const float t = fmod(
      (presentation_time - start_time_) * kSecondsPerNanosecond * kSpeed, 1.f);
  const float angle = t * M_PI * 2;

  mozart::client::Rectangle background_shape(session(), size().width,
                                             size().height);
  background_node_.SetShape(background_shape);
  background_node_.SetTranslation(
      (float[]){center_x, center_y, kBackgroundElevation});

  mozart::client::Rectangle square_shape(session(), square_size, square_size);
  square_node_.SetShape(square_shape);
  square_node_.SetTranslation((float[]){center_x, center_y, kSquareElevation});
  square_node_.SetRotation(
      (float[]){0.f, 0.f, sinf(angle * .5f), cosf(angle * .5f)});

  InvalidateScene();
}

}  // namespace examples
