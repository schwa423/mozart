// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/mozart/src/compositor/renderer_state.h"

#include "lib/ftl/logging.h"
#include "lib/ftl/strings/string_printf.h"

namespace compositor {

RendererState::RendererState(uint32_t id,
                             const std::string& label,
                             std::unique_ptr<Output> output)
    : id_(id),
      label_(label),
      output_(std::move(output)),
      scheduler_(output_.get()),
      weak_factory_(this) {
  FTL_DCHECK(output_);
}

RendererState::~RendererState() {}

bool RendererState::SetRootScene(SceneState* scene,
                                 uint32_t version,
                                 const mozart::Rect& viewport) {
  FTL_DCHECK(scene);

  if (root_scene_ != scene || root_scene_version_ != version ||
      !root_scene_viewport_.Equals(viewport)) {
    root_scene_ = scene;
    root_scene_version_ = version;
    root_scene_viewport_ = viewport;
    SetSnapshot(nullptr);
    return true;
  }
  return false;
}

bool RendererState::ClearRootScene() {
  if (root_scene_) {
    root_scene_ = nullptr;
    SetSnapshot(nullptr);
    return true;
  }
  return false;
}

void RendererState::SetSnapshot(const ftl::RefPtr<const Snapshot>& snapshot) {
  current_snapshot_ = snapshot;
  if (current_snapshot_ && !current_snapshot_->is_blocked())
    visible_snapshot_ = current_snapshot_;
}

std::string RendererState::FormattedLabel() {
  if (formatted_label_cache_.empty()) {
    formatted_label_cache_ =
        label_.empty() ? ftl::StringPrintf("<%d>", id_)
                       : ftl::StringPrintf("<%d:%s>", id_, label_.c_str());
  }
  return formatted_label_cache_;
}

std::ostream& operator<<(std::ostream& os, RendererState* renderer_state) {
  if (!renderer_state)
    return os << "null";
  return os << renderer_state->FormattedLabel();
}

}  // namespace compositor
