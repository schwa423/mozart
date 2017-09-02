// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_EXAMPLES_PAINT_PAINT_VIEW_H_
#define APPS_MOZART_EXAMPLES_PAINT_PAINT_VIEW_H_

#include <map>
#include <vector>

#include "apps/mozart/lib/view_framework/skia_view.h"
#include "lib/ftl/macros.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"

namespace examples {

class PaintView : public mozart::SkiaView {
 public:
  PaintView(mozart::ViewManagerPtr view_manager,
            fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request);
  ~PaintView() override;

 private:
  // |BaseView|:
  void OnSceneInvalidated(
      scenic::PresentationInfoPtr presentation_info) override;
  bool OnInputEvent(mozart::InputEventPtr event) override;

  void DrawContent(SkCanvas* canvas);
  SkPath CurrentPath(uint32_t pointer_id);

  std::map<uint32_t, std::vector<SkPoint>> points_;
  std::vector<SkPath> paths_;

  FTL_DISALLOW_COPY_AND_ASSIGN(PaintView);
};

}  // namespace examples

#endif  // APPS_MOZART_EXAMPLES_PAINT_PAINT_VIEW_H_
