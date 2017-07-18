// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_EXAMPLES_SPINNING_SQUARE_SPINNING_SQUARE_VIEW_H_
#define APPS_MOZART_EXAMPLES_SPINNING_SQUARE_SPINNING_SQUARE_VIEW_H_

#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/view_framework/base_view.h"
#include "lib/ftl/macros.h"

namespace examples {

class SpinningSquareView : public mozart::BaseView {
 public:
  SpinningSquareView(
      mozart::ViewManagerPtr view_manager,
      fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request);
  ~SpinningSquareView() override;

 private:
  // |BaseView|:
  void OnPropertiesChanged(mozart::ViewPropertiesPtr old_properties) override;
  void OnSceneInvalidated(
      mozart2::PresentationInfoPtr presentation_info) override;

  mozart::client::ShapeNode background_node_;
  mozart::client::ShapeNode square_node_;

  uint64_t start_time_ = 0u;

  FTL_DISALLOW_COPY_AND_ASSIGN(SpinningSquareView);
};

}  // namespace examples

#endif  // APPS_MOZART_EXAMPLES_SPINNING_SQUARE_SPINNING_SQUARE_VIEW_H_
