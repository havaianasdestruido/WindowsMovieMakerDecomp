---
sidebar_position: 3
title: HMREngine (3D/D2D Render Core)
description: The DirectX 11 rendering engine with an X3D-inspired scene graph, text rendering, and pattern meshes.
---

# HMREngine

**Location:** `src/MovieMakerCore/HMREngine/`

HMREngine ("HMR" = the original's internal engine prefix) is the rendering core: a
scene-graph engine over **D3D11 + D2D + DirectWrite**, with an X3D/VRML-inspired node
system. The original was recovered from RTTI: `X3DFieldNode` base class, typed field
system (`SFBool`, `SFVec3f`, `MFNode`, ...), and a family of grouping/appearance/geometry
nodes.

## Tree

```text
HMREngine/
  HMREngine.cpp/.h        # engine facade + (see note) SDK macro redefinition zone
  Engine.cpp/.h           # device/context ownership, frame orchestration
  RenderLoop.cpp/.h       # the render loop
  RenderingList.cpp/.h    # per-frame renderable collection
  ExecutionContext.cpp/.h # per-frame execution state
  DXResources/            # DX11 resource management (textures, buffers, RTVs)
  TextRender/             # DirectWrite text rendering pipeline
  PatternMesh/            # video transition geometry (see below)
  X3DReader / scene impls # X3DNodeImpls, IndexedFaceSet, Material, Texture nodes...
  X3DMath.h               # Vec2/3/4, Rotation4f, Matrix3f/4f, Rgb/Rgba, Frustum (on DirectXMath)
  Browser.cpp/.h          # scene browser/debug surface
  GridImpl, CoordinateImpl, LayerImpl, InterpolatorImpl, EventImpl,
  MetadataImpl, ErrHandler, AsyncWorker, ReloadImageWorker
  d3dx11compat.h/.cpp     # D3DX11 compatibility stub (SDK compatibility)
```

## The X3D node system

All nodes inherit `CComObjectRootEx` (COM reference counting) and derive from
`X3DFieldNode`, which owns the typed field system. Node families include:

- **Grouping:** Group, Transform, Switch, Billboard
- **Appearance:** Material, ImageTexture, Appearance
- **Geometry:** Shape, Box, Sphere, Cylinder, Cone, Plane, Mesh (`IndexedFaceSet`)
- **Sensors:** TouchSensor, TimeSensor, ProximitySensor
- **Layers:** Layer2D, Layer3D

`X3DReader` parses scene descriptions (JSON-object syntax) into the node graph — with
**hard caps: nesting depth 512, node count 100,000**, and depth-guarded recursive parsing
(hostile-input hardening; see [Security](../../reference/security.md)).

## Scene traversal

Per frame, `Engine` → `RenderLoop` walks the scene graph building a `RenderingList`.
Traversal (`Scene::TraverseNode`, `X3DChildNodeImpl::Traverse`, `GroupNodeImpl::Traverse`,
`SwitchNodeImpl::TraverseActive`, `EnumerateNodes::EnumerateRecursive`) is **depth-capped
at 512** to prevent stack exhaustion on hostile graphs — a hardening pass, not original
behavior, and one of the few deliberate divergences (documented in the module README).

## PatternMesh and transitions

`PatternMesh/` generates the wipe/reveal geometry used by video transitions. Transition
animations themselves live in **WLXPipetran**; HMREngine renders whatever mesh/texture
state they produce.

## D3DX11 compatibility zone

The original linked the deprecated D3DX11 Effect Framework. `d3dx11compat.h/.cpp`
re-implements the needed surface (`ID3DX11Effect`, `ID3DX11EffectTechnique`,
`ID3DX11EffectPass`, ...) over raw D3D11, returning `E_NOTIMPL` for unused paths. See
[SDK Compatibility](../../reference/sdk-compatibility.md).

:::warning Macro redefinition zone

`HMREngine.h` (lines ~111–182) temporarily **redefines Windows SDK error macros** and
restores them — matching the original binary's layout. Do not add `#include` directives
inside that span; it breaks the restore logic
([Quirk #5](../../methodology/quirks.md#5-hmrengine-redefines-windows-sdk-error-macros)).

:::

## Error handling — preserved quirks

- HMREngine **silently swallows non-fatal failures** (log-and-continue) — original
  behavior, [Quirk #21](../../methodology/quirks.md#21-hmrengine-error-handling-silently-swallows-failures).
- **118 dummy texture codec classes** exist purely for RTTI parity
  ([Quirk #13](../../methodology/quirks.md#13-118-dummy-texture-codec-classes)).
- Local classes nested inside methods appear in RTTI
  ([Quirk #14](../../methodology/quirks.md#14-local-classes-nested-inside-methods-rtti)).
