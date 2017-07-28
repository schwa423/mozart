# Shady

TODO(MZ-??): unimplemented.

Shady is intended to to generate procedural Scenic materials that are beyond the
scope of what can be achieved by the Shadertoy service (which remains valuable
as a simple example).

For example, when using a Shadertoy to generate a material that is applied to a
3D mesh, there is no way to provide the fragment shader with parameters that
model an object's geometry (e.g. surface normal), nor its lighting environment
(e.g. the vectors toward the eye and lights).

TODO(MZ-??): high-level architecture for Shady.

In previous discussions, we have tended to talk about Shady as a separate
service.  Unfortunately, this approach makes it difficult to support the
canonical use-case above.

In my evolving understanding, it would be better to structure Shady as a set of
Scenic Resource types and their associated Ops; these would be implemented
directly by the scene manager.  

In this scenario, a separate Shady service could still play a role as a testbed
and exemplar for:

1) Higher-level APIs that are more expressive and/or simpler to use than the
"assembly language" level of the Ops supported by the scene manager.

2) Multi-device coordination (Croquet), persistence (Ledger), and composition
with other services (Sketchy and StretchyBouncy).
