#pragma once
// X3DNodeImpls.h - Root header for all X3D node implementation bridge types

#include "../HMREngine.h"
#include "../X3DNodes.h"
#include "../X3DFieldTypes.h"
#include "../X3DMath.h"
#include <map>
#include <string>
#include <vector>
#include <functional>

namespace HMREngine
{
    // --- Forward declarations ---
    class X3DNodeImpl;
    class X3DChildNodeImpl;
    class X3DChildObjectImpl;
    class X3DBoundedObjectImpl;
    class X3DGroupingNodeImpl;
    class X3DShapeNodeImpl;
    class X3DGeometryNodeImpl;
    class X3DComposedGeometryNodeImpl;
    class X3DTextureNodeImpl;
    class X3DTexture2DNodeImpl;
    class X3DUrlObjectImpl;
    class X3DMaterialNodeImpl;
    class X3DAppearanceNodeImpl;
    class X3DSensorNodeImpl;
    class X3DTimeDependentObjectImpl;
    class X3DLayerNodeImpl;
    class X3DViewportNodeImpl;
    class X3DBindableNodeImpl;
    class X3DSoundNodeImpl;
    class X3DSoundSourceObjectImpl;
    class X3DSoundSourceNodeImpl;
    class X3DAnimatedShaderNodeImpl;
    class X3DShaderNodeImpl;
    class X3DTextNodeImpl;
    class X3DVertexAttributeNodeImpl;
    class X3DMetadataObjectImpl;
    class TextureTransformContainerImpl;
    class MotionTextureNodeImpl;
    class ShaderSetNodeImpl;
    class TexturePropertiesNodeImpl;
    class FieldCollection;
    class FieldImpl;
    class IsActiveBase;
    class TransparentObject;

    // --- Root header includes for all impl groups ---
    #include "X3DNodeImplBase.h"
    #include "X3DGroupingImpls.h"
    #include "X3DAppearanceImpls.h"
    #include "X3DLayerImpls.h"
    #include "X3DSensorImpls.h"
