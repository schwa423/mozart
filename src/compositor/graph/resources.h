// Copyright 2015 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_MOZART_SRC_COMPOSITOR_GRAPH_RESOURCES_H_
#define APPS_MOZART_SRC_COMPOSITOR_GRAPH_RESOURCES_H_

#include "apps/mozart/services/composition/resources.fidl.h"
#include "apps/mozart/src/compositor/render/render_image.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/ref_counted.h"
#include "lib/mtl/vmo/shared_vmo.h"

namespace compositor {

// Base class for resources in a scene graph.
//
// Instances of this class are immutable and reference counted so they may
// be shared by multiple versions of the same scene.
class Resource : public ftl::RefCountedThreadSafe<Resource> {
 public:
  enum class Type { kScene, kImage };

  Resource();

  // Gets the resource type.
  virtual Type type() const = 0;

 protected:
  FRIEND_REF_COUNTED_THREAD_SAFE(Resource);
  virtual ~Resource();

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(Resource);
};

// A resource which represents a reference to a specified scene.
class SceneResource : public Resource {
 public:
  explicit SceneResource(const mozart::SceneToken& scene_token);

  Type type() const override;

  const mozart::SceneToken& scene_token() const { return scene_token_; }

 protected:
  ~SceneResource() override;

 private:
  mozart::SceneToken scene_token_;

  FTL_DISALLOW_COPY_AND_ASSIGN(SceneResource);
};

// A resource which represents a reference to a specified image.
class ImageResource : public Resource {
 public:
  explicit ImageResource(ftl::RefPtr<RenderImage> image);

  Type type() const override;

  // The referenced image, never null.
  const ftl::RefPtr<RenderImage>& image() const { return image_; }

 protected:
  ~ImageResource() override;

 private:
  ftl::RefPtr<RenderImage> image_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ImageResource);
};

}  // namespace compositor

#endif  // APPS_MOZART_SRC_COMPOSITOR_GRAPH_RESOURCES_H_
