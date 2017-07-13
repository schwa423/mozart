// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/mozart/src/scene/resources/resource.h"
#include "escher/material/material.h"

namespace mozart {
namespace scene {

class ImageBase;
using ImageBasePtr = ftl::RefPtr<ImageBase>;

class Material;
using MaterialPtr = ftl::RefPtr<Material>;

class Material : public Resource {
 public:
  static const ResourceTypeInfo kTypeInfo;

  Material(Session* session, ResourceId id);

  void SetColor(float red, float green, float blue, float alpha);
  void SetTexture(ImageBasePtr texture_image);

  float red() const { return escher_material_->color().x; }
  float green() const { return escher_material_->color().y; }
  float blue() const { return escher_material_->color().z; }
  float alpha() const {
    FTL_CHECK(false);
    return 0.f;
  }
  const ImageBasePtr& texture_image() const { return texture_; }
  const escher::MaterialPtr& escher_material() const {
    return escher_material_;
  }

  void Accept(class ResourceVisitor* visitor) override;

  // Called at presentation time to allow ImagePipes to update current image.
  void UpdateEscherMaterial();

 private:
  escher::MaterialPtr escher_material_;
  ImageBasePtr texture_;
};

}  // namespace scene
}  // namespace mozart
