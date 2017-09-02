// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/examples/shadertoy/service/glm_hack.h"
#include "apps/mozart/examples/shadertoy/service/services/shadertoy.fidl.h"
#include "apps/mozart/services/images/image_pipe.fidl.h"
#include "apps/mozart/services/views/view_token.fidl.h"
#include "escher/escher.h"
#include "escher/util/stopwatch.h"
#include "lib/ftl/memory/ref_counted.h"
#include "lib/ftl/memory/weak_ptr.h"

namespace shadertoy {

class Compiler;
class Pipeline;
class Renderer;
class App;
using PipelinePtr = ftl::RefPtr<Pipeline>;

// Core implementation of the Shadertoy API.  Subclasses must provide some
// functionality, such as the method for obtaining a framebuffer to render into.
class ShadertoyState : public ftl::RefCountedThreadSafe<ShadertoyState> {
 public:
  // Factory constructor.
  static ftl::RefPtr<ShadertoyState> NewForImagePipe(
      App* app,
      ::fidl::InterfaceHandle<scenic::ImagePipe> image_pipe);

  // Factory constructor.
  static ftl::RefPtr<ShadertoyState> NewForView(
      App* app,
      ::fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request,
      bool handle_input_events);

  virtual ~ShadertoyState();

  void SetPaused(bool paused);

  void SetShaderCode(
      std::string glsl,
      const mozart::example::Shadertoy::SetShaderCodeCallback& callback);

  void SetResolution(uint32_t width, uint32_t height);

  void SetMouse(glm::vec4 i_mouse);

  void SetImage(uint32_t channel,
                ::fidl::InterfaceRequest<scenic::ImagePipe> request);

 protected:
  explicit ShadertoyState(App* app);

  // Tell the app to close the connection to this Shadertoy, and destroy it.
  void Close();

  // Subclasses must call this from DrawFrame().
  void OnFramePresented(const scenic::PresentationInfoPtr& info);

  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  escher::Escher* escher() const { return escher_; }
  Renderer* renderer() const { return renderer_; }
  const PipelinePtr& pipeline() const { return pipeline_; }
  escher::Texture* channel0() const { return nullptr; }
  escher::Texture* channel1() const { return nullptr; }
  escher::Texture* channel2() const { return nullptr; }
  escher::Texture* channel3() const { return nullptr; }
  glm::vec4 i_mouse() const { return i_mouse_; }
  ftl::WeakPtrFactory<ShadertoyState>* weak_ptr_factory() {
    return &weak_ptr_factory_;
  }

 private:
  // Subclasses must implement this, and call OnFramePresented() from it.
  virtual void DrawFrame(uint64_t presentation_time, float animation_time) = 0;

  // Requests a frame to be drawn.
  void RequestFrame(uint64_t presentation_time);

  // Subclasses must implement this to react when the resolution changes.
  virtual void OnSetResolution() = 0;

  static constexpr uint32_t kMaxWidth = 2048;
  static constexpr uint32_t kMaxHeight = 2048;

  App* const app_;
  escher::Escher* const escher_;
  Compiler* const compiler_;
  Renderer* const renderer_;
  ftl::WeakPtrFactory<ShadertoyState> weak_ptr_factory_;
  PipelinePtr pipeline_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  glm::vec4 i_mouse_ = {0, 0, 0, 0};
  bool is_paused_ = true;
  bool is_drawing_ = false;
  escher::Stopwatch stopwatch_;
};

}  // namespace shadertoy
