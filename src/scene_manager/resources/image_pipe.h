// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <mx/event.h>
#include <mx/vmo.h>

#include <deque>

#include "apps/mozart/services/images/image_pipe.fidl.h"
#include "apps/mozart/src/scene_manager/acquire_fence.h"
#include "apps/mozart/src/scene_manager/resources/image.h"
#include "apps/mozart/src/scene_manager/resources/image_base.h"
#include "apps/mozart/src/scene_manager/resources/image_pipe.h"
#include "apps/mozart/src/scene_manager/resources/image_pipe_handler.h"
#include "apps/mozart/src/scene_manager/resources/resource.h"
#include "apps/mozart/src/scene_manager/resources/resource_map.h"
#include "lib/ftl/memory/weak_ptr.h"
#include "lib/mtl/tasks/message_loop.h"
#include "lib/mtl/tasks/message_loop_handler.h"

namespace scene_manager {

class ImagePipe;
using ImagePipePtr = ftl::RefPtr<ImagePipe>;

class ImagePipe : public ImageBase {
 public:
  static const ResourceTypeInfo kTypeInfo;

  ImagePipe(Session* session, mozart::ResourceId id);
  ImagePipe(Session* session,
            mozart::ResourceId id,
            ::fidl::InterfaceRequest<mozart2::ImagePipe> request);

  // Called by |ImagePipeHandler|, part of |ImagePipe| interface.
  void AddImage(uint32_t image_id,
                mozart2::ImageInfoPtr image_info,
                mx::vmo memory,
                mozart2::MemoryType memory_type,
                uint64_t memory_offset);
  void RemoveImage(uint32_t image_id);
  // TODO(MZ-152): Add Presentation time to image_pipe.fidl.
  void PresentImage(uint32_t image_id,
                    uint64_t presentation_time,
                    mx::event acquire_fence,
                    mx::event release_fence,
                    const mozart2::ImagePipe::PresentImageCallback& callback);

  void Accept(class ResourceVisitor* visitor) override;

  // Update to use the most current frame for the specified presentation time.
  // Called before rendering a frame using this ImagePipe.  Return true if the
  // current Image changed since the last time Update() was called, and false
  // otherwise.
  bool Update(uint64_t presentation_time, uint64_t presentation_interval);

  // Returns the image that should be presented at the current time. Can be
  // null.
  const escher::ImagePtr& GetEscherImage() override;

  // Returns true if the connection to the ImagePipe has not closed.
  bool is_valid() { return is_valid_; };

 private:
  friend class ImagePipeHandler;

  // Called when the image pipe connection is closed.
  void OnConnectionError();

  // Called when we want to close the connection ourselves. Cleans up resources
  // and schedules a new frame update.
  void CloseConnectionAndCleanUp();

  // Virtual so that test subclasses can override.
  virtual ImagePtr CreateImage(Session* session,
                               MemoryPtr memory,
                               const mozart2::ImageInfoPtr& image_info,
                               uint64_t memory_offset,
                               ErrorReporter* error_reporter);

  ftl::WeakPtrFactory<ImagePipe> weak_ptr_factory_;

  // A |Frame| stores the arguments passed to a particular invocation of
  // Present().
  struct Frame {
    mozart::ResourceId image_id;
    uint64_t presentation_time;
    std::unique_ptr<AcquireFence> acquire_fence;
    mx::event release_fence;

    // Callback to report when the update has been applied in response to
    // an invocation of |ImagePipe.PresentImage()|.
    mozart2::ImagePipe::PresentImageCallback present_image_callback;
  };
  std::deque<Frame> frames_;
  std::unique_ptr<ImagePipeHandler> handler_;

  mozart::ResourceId current_image_id_ = 0;
  ImagePtr current_image_;
  mx::event current_release_fence_;

  ResourceMap images_;
  bool is_valid_ = true;

  FTL_DISALLOW_COPY_AND_ASSIGN(ImagePipe);
};

}  // namespace scene_manager
