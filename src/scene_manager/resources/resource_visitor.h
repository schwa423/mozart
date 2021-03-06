// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

namespace scene_manager {

class Import;
class GpuMemory;
class HostMemory;
class Image;
class ImagePipe;
class EntityNode;
class Node;
class ShapeNode;
class Scene;
class CircleShape;
class RectangleShape;
class RoundedRectangleShape;
class Material;
class DisplayCompositor;
class LayerStack;
class Layer;
class Camera;
class Renderer;
class Scene;
class DirectionalLight;

class ResourceVisitor {
 public:
  // Memory resources.
  virtual void Visit(GpuMemory* r) = 0;
  virtual void Visit(HostMemory* r) = 0;
  virtual void Visit(Image* r) = 0;
  virtual void Visit(ImagePipe* r) = 0;

  // Nodes.
  virtual void Visit(EntityNode* r) = 0;
  virtual void Visit(ShapeNode* r) = 0;

  // Shapes.
  virtual void Visit(CircleShape* r) = 0;
  virtual void Visit(RectangleShape* r) = 0;
  virtual void Visit(RoundedRectangleShape* r) = 0;

  // Materials.
  virtual void Visit(Material* r) = 0;

  // Layers.
  virtual void Visit(DisplayCompositor* r) = 0;
  virtual void Visit(LayerStack* r) = 0;
  virtual void Visit(Layer* r) = 0;

  // Scene, camera, lighting.
  virtual void Visit(Scene* r) = 0;
  virtual void Visit(Camera* r) = 0;
  virtual void Visit(Renderer* r) = 0;
  virtual void Visit(DirectionalLight* r) = 0;

  // Imported resources.
  virtual void Visit(Import* r) = 0;
};

}  // namespace scene_manager
