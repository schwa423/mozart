// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_EXAMPLES_SHAPES_SHAPES_VIEW_H_
#define APPS_MOZART_EXAMPLES_SHAPES_SHAPES_VIEW_H_

#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/view_framework/base_view.h"
#include "lib/ftl/macros.h"

class SkCanvas;

namespace examples {

class ShapesView : public mozart::BaseView {
 public:
  ShapesView(mozart::ViewManagerPtr view_manager,
             fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request);

  ~ShapesView() override;

 private:
  // |BaseView|:
  void OnSceneInvalidated(
      mozart2::PresentationInfoPtr presentation_info) override;

  mozart::client::ShapeNode background_node_;
  mozart::client::ShapeNode card_node_;
  mozart::client::ShapeNode circle_node_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ShapesView);
};

}  // namespace examples

#endif  // APPS_MOZART_EXAMPLES_SHAPES_SHAPES_VIEW_H_
