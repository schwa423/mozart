# Stretchy

TODO(MZ-??): unimplemented.

Deformable geometry experiments.  The goal is to look for simple, powerful
primitives to be exposed by Scenic, which can be mediated by helper modules to
provide higher-level APIs.

A promising candidate is bone-weighted mesh skinning.  This is an efficient
approach that is most commonly used in skeletal animation to deform the skin of
a character model based on the nearest bones.  However, recent research shows
how this approach can be used to accelerate more sophisticated deformation
algorithms.  The general idea is to model the deformation characteristics of
a shape using a technique from the "as rigid as possible" family, and to run an
optimization algorithm that "bakes" these characteristics into a bone-weighted
mesh format that can be directly used by Scenic.

## Background reading:

"Animato: 2D shape deformation and animation on mobile devices", SIGGRAPH ASIA
2016.
