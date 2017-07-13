// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene/resources/gpu_memory.h"
#include "apps/mozart/src/scene/resources/host_memory.h"
#include "apps/mozart/src/scene/resources/image_base.h"
#include "apps/mozart/src/scene/resources/memory.h"
#include "apps/mozart/src/scene/resources/resource.h"
#include "escher/renderer/image.h"

namespace mozart {
namespace scene {

class Image;
using ImagePtr = ftl::RefPtr<Image>;

class Image : public ImageBase {
 public:
  static const ResourceTypeInfo kTypeInfo;

  // Create Image given a MemoryPtr, mozart2::ImageInfoPtr, and memory_offset.
  //
  // |session| is the Session that this image can be referenced from.
  // |memory| is the memory that is associated with this image.
  // |args| specifies size, format, and other properties.
  // |error_reporter| is used to log any errors so they can be seen by the
  // caller.
  //
  // Returns the created Image, or nullptr if there was an error.
  static ImagePtr New(Session* session,
                      ResourceId id,
                      MemoryPtr memory,
                      const mozart2::ImageInfoPtr& image_info,
                      uint64_t memory_offset,
                      ErrorReporter* error_reporter);

  static ImagePtr NewForTesting(Session* session,
                                ResourceId id,
                                escher::ResourceManager* image_owner,
                                MemoryPtr host_memory);

  void Accept(class ResourceVisitor* visitor) override;

  const escher::ImagePtr& GetEscherImage() override { return image_; }

 private:
  // Create an Image object from a VkImage.
  // |session| is the Session that this image can be referenced from.
  // |image_info| specifies size, format, and other properties.
  // |vk_image| is the VkImage, whose lifetime is now controlled by this
  // object. |memory| is the GPU memory that is associated with this image.
  Image(Session* session,
        ResourceId id,
        GpuMemoryPtr memory,
        escher::ImageInfo image_info,
        vk::Image vk_image_);

  // Create an Image object from a escher::Image.
  // |session| is the Session that this image can be referenced from.
  // |image| is the escher::Image that is being wrapped.
  // |memory| is the host memory that is associated with this image.
  //
  // TODO: We might not want to hold on to the memory since we're uploading
  // its contents to the GPU and using the uploaded copy.
  Image(Session* session,
        ResourceId id,
        MemoryPtr memory,
        escher::ImagePtr image);

  MemoryPtr memory_;
  escher::ImagePtr image_;
};

}  // namespace scene
}  // namespace mozart
