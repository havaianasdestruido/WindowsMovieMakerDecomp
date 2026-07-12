/*
 * ThemeTemplates.h
 *
 * X3D template system for themes. Provides the base template classes,
 * X3D scene templates, template placeholders/properties/sockets, and
 * handlers for effect and extent socket mapping.
 *
 * RTTI classes:
 *   ?AVBaseTemplate@@, ?AVBaseX3DTemplate@@, ?AVCompositeX3DTemplate@@
 *   ?AVThemeX3DTemplate@@, ?AVEffectX3DTemplate@@, ?AVExtentX3DTemplate@@
 *   ?AVTemplatePlaceholder@@, ?AVTemplateProperty@@
 *   ?AVTemplateSocket@@, ?AVTemplateSocketsParser@@
 *   ?AVExtentSocketsHandler@@, ?AVEffectTemplateHandler@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef THEMES_TEMPLATES_H
#define THEMES_TEMPLATES_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// Template socket types
// ============================================================================
enum TemplateSocketType
{
    TemplateSocketTypeNone          = 0,
    TemplateSocketTypeVideo         = 1,    // video media socket
    TemplateSocketTypeAudio         = 2,    // audio media socket
    TemplateSocketTypeImage         = 3,    // image media socket
    TemplateSocketTypeText          = 4,    // text overlay socket
    TemplateSocketTypeTransition    = 5,    // transition socket
    TemplateSocketTypeEffect        = 6,    // effect socket
    TemplateSocketTypeBackground    = 7     // background socket
};

// ============================================================================
// TemplatePlaceholder
// ============================================================================
// Represents a placeholder within an X3D template scene. Placeholders mark
// positions where media, text, or effects can be inserted during rendering.
//
class STORYBOARD_API TemplatePlaceholder
{
public:
    TemplatePlaceholder();
    ~TemplatePlaceholder();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Type
    TemplateSocketType GetType() const throw();
    void SetType(TemplateSocketType type) throw();

    // Transform (position/rotation/scale in 3D space)
    float GetPositionX() const throw();
    float GetPositionY() const throw();
    float GetPositionZ() const throw();
    void SetPosition(float x, float y, float z) throw();

    float GetRotationX() const throw();
    float GetRotationY() const throw();
    float GetRotationZ() const throw();
    void SetRotation(float x, float y, float z) throw();

    float GetScaleX() const throw();
    float GetScaleY() const throw();
    float GetScaleZ() const throw();
    void SetScale(float x, float y, float z) throw();

    // Bounds (normalized 0.0-1.0)
    float GetBoundsLeft() const throw();
    float GetBoundsTop() const throw();
    float GetBoundsRight() const throw();
    float GetBoundsBottom() const throw();
    void SetBounds(float left, float top, float right, float bottom) throw();

    // Duration (hundred-nanoseconds, 0 = inherit from parent)
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Default value
    ATL::CString GetDefaultValue() const;
    void SetDefaultValue(LPCWSTR pszValue);

    // Required flag
    bool IsRequired() const throw();
    void SetRequired(bool fRequired) throw();

private:
    ATL::CString        m_strId;
    ATL::CString        m_strName;
    TemplateSocketType  m_type;
    float               m_posX;
    float               m_posY;
    float               m_posZ;
    float               m_rotX;
    float               m_rotY;
    float               m_rotZ;
    float               m_scaleX;
    float               m_scaleY;
    float               m_scaleZ;
    float               m_boundsLeft;
    float               m_boundsTop;
    float               m_boundsRight;
    float               m_boundsBottom;
    LONGLONG            m_llDurationHns;
    ATL::CString        m_strDefaultValue;
    bool                m_fRequired;
};

// ============================================================================
// TemplateProperty
// ============================================================================
// A key-value property on a template or template element. Properties control
// rendering parameters, animation settings, and other template metadata.
//
class STORYBOARD_API TemplateProperty
{
public:
    TemplateProperty();
    TemplateProperty(LPCWSTR pszKey, LPCWSTR pszValue);
    ~TemplateProperty();

    ATL::CString GetKey() const;
    void SetKey(LPCWSTR pszKey);

    ATL::CString GetValue() const;
    void SetValue(LPCWSTR pszValue);

    // Typed accessors
    int GetIntValue() const;
    void SetIntValue(int nValue);

    float GetFloatValue() const;
    void SetFloatValue(float flValue);

    bool GetBoolValue() const;
    void SetBoolValue(bool fValue);

    // Inherited flag (from parent template)
    bool IsInherited() const throw();
    void SetInherited(bool fInherited) throw();

private:
    ATL::CString    m_strKey;
    ATL::CString    m_strValue;
    bool            m_fInherited;
};

// ============================================================================
// TemplateSocket
// ============================================================================
// Defines a connection point on an X3D template where media or effects
// can be attached. Sockets have a type, position, and constraints that
// determine what can be connected.
//
class STORYBOARD_API TemplateSocket
{
public:
    TemplateSocket();
    ~TemplateSocket();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Socket type
    TemplateSocketType GetType() const throw();
    void SetType(TemplateSocketType type) throw();

    // Placeholder reference
    ATL::CString GetPlaceholderId() const;
    void SetPlaceholderId(LPCWSTR pszPlaceholderId);

    // Constraints
    bool AllowsVideo() const throw();
    bool AllowsAudio() const throw();
    bool AllowsImage() const throw();
    bool AllowsText() const throw();

    void SetAllowedTypes(DWORD dwMask) throw();
    DWORD GetAllowedTypes() const throw();

    // Index (position in the template)
    DWORD GetIndex() const throw();
    void SetIndex(DWORD dwIndex) throw();

    // Connected state
    bool IsConnected() const throw();
    void SetConnected(bool fConnected) throw();

    // Properties on this socket
    size_t GetPropertyCount() const throw();
    TemplateProperty* GetProperty(size_t nIndex);
    const TemplateProperty* GetProperty(size_t nIndex) const;
    TemplateProperty* FindProperty(LPCWSTR pszKey);
    size_t AddProperty(const TemplateProperty& prop);
    void RemoveProperty(size_t nIndex);
    void RemoveAllProperties();

private:
    ATL::CString                m_strId;
    ATL::CString                m_strName;
    TemplateSocketType          m_type;
    ATL::CString                m_strPlaceholderId;
    DWORD                       m_dwAllowedTypes;
    DWORD                       m_dwIndex;
    bool                        m_fConnected;
    ATL::CAtlArray<TemplateProperty> m_arrProperties;
};

// ============================================================================
// TemplateSocketsParser
// ============================================================================
// Parses socket definitions from XML theme files. Reads <sockets> elements
// and creates TemplateSocket/TemplatePlaceholder objects.
//
class STORYBOARD_API TemplateSocketsParser
{
public:
    TemplateSocketsParser();
    ~TemplateSocketsParser();

    // Parse sockets from an XML reader positioned at a <sockets> element
    HRESULT Parse(IXmlReader* pReader,
                  ATL::CAtlArray<TemplateSocket>& sockets,
                  ATL::CAtlArray<TemplatePlaceholder>& placeholders);

    // Parse a single <socket> element
    HRESULT ParseSocket(IXmlReader* pReader, TemplateSocket& socket);

    // Parse a single <placeholder> element
    HRESULT ParsePlaceholder(IXmlReader* pReader, TemplatePlaceholder& placeholder);

    // Error info
    HRESULT GetLastError() const throw();
    ATL::CString GetLastErrorMessage() const;

private:
    HRESULT ParseSocketAttributes(IXmlReader* pReader, TemplateSocket& socket);
    HRESULT ParsePlaceholderAttributes(IXmlReader* pReader, TemplatePlaceholder& placeholder);
    HRESULT ParseSocketProperties(IXmlReader* pReader, TemplateSocket& socket);

    HRESULT     m_hrLastError;
    ATL::CString m_strLastError;
};

// ============================================================================
// BaseTemplate
// ============================================================================
// Abstract base class for all template types. Provides common identity,
// property storage, and serialization interface.
//
class STORYBOARD_API BaseTemplate
{
public:
    BaseTemplate();
    virtual ~BaseTemplate();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Type name (for RTTI-like dispatch)
    virtual ATL::CString GetTemplateType() const;

    // Properties
    size_t GetPropertyCount() const throw();
    TemplateProperty* GetProperty(size_t nIndex);
    const TemplateProperty* GetProperty(size_t nIndex) const;
    TemplateProperty* FindProperty(LPCWSTR pszKey);
    size_t AddProperty(const TemplateProperty& prop);
    void RemoveProperty(size_t nIndex);
    void RemoveAllProperties();

    // Sockets
    size_t GetSocketCount() const throw();
    TemplateSocket* GetSocket(size_t nIndex);
    const TemplateSocket* GetSocket(size_t nIndex) const;
    TemplateSocket* FindSocket(LPCWSTR pszId);
    size_t AddSocket(const TemplateSocket& socket);
    void RemoveSocket(size_t nIndex);
    void RemoveAllSockets();

    // Placeholders
    size_t GetPlaceholderCount() const throw();
    TemplatePlaceholder* GetPlaceholder(size_t nIndex);
    const TemplatePlaceholder* GetPlaceholder(size_t nIndex) const;
    TemplatePlaceholder* FindPlaceholder(LPCWSTR pszId);
    size_t AddPlaceholder(const TemplatePlaceholder& placeholder);
    void RemovePlaceholder(size_t nIndex);
    void RemoveAllPlaceholders();

    // Serialization
    virtual HRESULT LoadFromXml(IXmlReader* pReader);
    virtual HRESULT SaveToXml(IXmlWriter* pWriter);

    // Duration (hundred-nanoseconds)
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

protected:
    ATL::CString    m_strId;
    ATL::CString    m_strName;
    LONGLONG        m_llDurationHns;

    ATL::CAtlArray<TemplateProperty>        m_arrProperties;
    ATL::CAtlArray<TemplateSocket>          m_arrSockets;
    ATL::CAtlArray<TemplatePlaceholder>     m_arrPlaceholders;
};

// ============================================================================
// BaseX3DTemplate
// ============================================================================
// Base class for X3D-based templates. Adds X3D scene graph reference and
// rendering parameters specific to X3D content.
//
class STORYBOARD_API BaseX3DTemplate : public BaseTemplate
{
public:
    BaseX3DTemplate();
    virtual ~BaseX3DTemplate();

    // X3D scene file path
    ATL::CString GetX3dScenePath() const;
    void SetX3dScenePath(LPCWSTR pszPath);

    // X3D scene data (in-memory)
    const BYTE* GetX3dSceneData() const throw();
    DWORD GetX3dSceneDataSize() const throw();
    HRESULT SetX3dSceneData(const BYTE* pData, DWORD dwSize);

    // Render dimensions
    UINT GetRenderWidth() const throw();
    UINT GetRenderHeight() const throw();
    void SetRenderDimensions(UINT cx, UINT cy) throw();

    // Frame rate
    DWORD GetFrameRate() const throw();
    void SetFrameRate(DWORD dwFrameRate) throw();

    // Animation keyframes
    DWORD GetKeyframeCount() const throw();
    HRESULT AddKeyframe(LONGLONG llTimeHns, LPCWSTR pszTargetId, LPCWSTR pszProperty, LPCWSTR pszValue);
    HRESULT ClearKeyframes();

    virtual ATL::CString GetTemplateType() const override;
    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

protected:
    ATL::CString    m_strX3dScenePath;
    ATL::CAtlArray<BYTE> m_arrX3dSceneData;
    UINT            m_uRenderWidth;
    UINT            m_uRenderHeight;
    DWORD           m_dwFrameRate;

    // Keyframe data
    struct Keyframe
    {
        LONGLONG    llTimeHns;
        ATL::CString strTargetId;
        ATL::CString strProperty;
        ATL::CString strValue;
    };
    ATL::CAtlArray<Keyframe> m_arrKeyframes;
};

// ============================================================================
// CompositeX3DTemplate
// ============================================================================
// A composite X3D template that combines multiple sub-templates into a
// single rendered scene. Used for complex themes with layered effects.
//
class STORYBOARD_API CompositeX3DTemplate : public BaseX3DTemplate
{
public:
    CompositeX3DTemplate();
    virtual ~CompositeX3DTemplate();

    // Sub-template management
    size_t GetSubTemplateCount() const throw();
    BaseX3DTemplate* GetSubTemplate(size_t nIndex);
    const BaseX3DTemplate* GetSubTemplate(size_t nIndex) const;
    size_t AddSubTemplate(BaseX3DTemplate* pTemplate);
    void RemoveSubTemplate(size_t nIndex);
    void RemoveAllSubTemplates();

    // Layer ordering
    void MoveSubTemplate(size_t nIndexFrom, size_t nIndexTo);

    // Sub-template lookup by placeholder
    BaseX3DTemplate* FindSubTemplateForPlaceholder(LPCWSTR pszPlaceholderId);

    virtual ATL::CString GetTemplateType() const override;
    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CAtlArray<BaseX3DTemplate*> m_arrSubTemplates;
};

// ============================================================================
// ThemeX3DTemplate
// ============================================================================
// X3D template for an entire theme. Contains the scene definition for
// rendering theme intros, mids, and outros as 3D scenes.
//
class STORYBOARD_API ThemeX3DTemplate : public BaseX3DTemplate
{
public:
    ThemeX3DTemplate();
    virtual ~ThemeX3DTemplate();

    // Section association
    enum SectionType
    {
        SectionIntro     = 0,
        SectionMid       = 1,
        SectionOutro     = 2,
        SectionFull      = 3
    };

    SectionType GetSectionType() const throw();
    void SetSectionType(SectionType type) throw();

    // Background model path
    ATL::CString GetBackgroundModelPath() const;
    void SetBackgroundModelPath(LPCWSTR pszPath);

    // Camera settings
    float GetCameraFov() const throw();
    void SetCameraFov(float flFov) throw();

    float GetCameraNear() const throw();
    void SetCameraNear(float flNear) throw();

    float GetCameraFar() const throw();
    void SetCameraFar(float flFar) throw();

    // Lighting
    DWORD GetAmbientColor() const throw();
    void SetAmbientColor(DWORD dwColor) throw();

    DWORD GetDirectionalColor() const throw();
    void SetDirectionalColor(DWORD dwColor) throw();

    virtual ATL::CString GetTemplateType() const override;
    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    SectionType     m_sectionType;
    ATL::CString    m_strBackgroundModelPath;
    float           m_flCameraFov;
    float           m_flCameraNear;
    float           m_flCameraFar;
    DWORD           m_dwAmbientColor;
    DWORD           m_dwDirectionalColor;
};

// ============================================================================
// EffectX3DTemplate
// ============================================================================
// X3D template for a specific visual effect. Defines the 3D scene used
// to render an effect (e.g., pan/zoom, vignette, film grain).
//
class STORYBOARD_API EffectX3DTemplate : public BaseX3DTemplate
{
public:
    EffectX3DTemplate();
    virtual ~EffectX3DTemplate();

    // Effect type this template renders
    DWORD GetEffectType() const throw();
    void SetEffectType(DWORD dwType) throw();

    // Shader paths
    ATL::CString GetVertexShaderPath() const;
    void SetVertexShaderPath(LPCWSTR pszPath);

    ATL::CString GetPixelShaderPath() const;
    void SetPixelShaderPath(LPCWSTR pszPath);

    // Texture paths
    size_t GetTextureCount() const throw();
    ATL::CString GetTexturePath(size_t nIndex) const;
    void AddTexturePath(LPCWSTR pszPath);
    void RemoveAllTextures();

    // Blend mode
    DWORD GetBlendMode() const throw();
    void SetBlendMode(DWORD dwMode) throw();

    virtual ATL::CString GetTemplateType() const override;
    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    DWORD           m_dwEffectType;
    ATL::CString    m_strVertexShaderPath;
    ATL::CString    m_strPixelShaderPath;
    ATL::CAtlArray<ATL::CString> m_arrTexturePaths;
    DWORD           m_dwBlendMode;
};

// ============================================================================
// ExtentX3DTemplate
// ============================================================================
// X3D template for an individual extent (media item) on the timeline.
// Defines how a single photo or video clip is rendered within the theme.
//
class STORYBOARD_API ExtentX3DTemplate : public BaseX3DTemplate
{
public:
    ExtentX3DTemplate();
    virtual ~ExtentX3DTemplate();

    // Media socket id
    ATL::CString GetMediaSocketId() const;
    void SetMediaSocketId(LPCWSTR pszId);

    // Entrance animation
    ATL::CString GetEntranceAnimation() const;
    void SetEntranceAnimation(LPCWSTR pszAnimation);

    // Exit animation
    ATL::CString GetExitAnimation() const;
    void SetExitAnimation(LPCWSTR pszAnimation);

    // Duration mode
    enum DurationMode
    {
        DurationModeFixed      = 0,
        DurationModeFitToMedia = 1,
        DurationModeFitToMusic = 2
    };

    DurationMode GetDurationMode() const throw();
    void SetDurationMode(DurationMode mode) throw();

    // Aspect ratio handling
    enum AspectRatioMode
    {
        AspectRatioCrop        = 0,
        AspectRatioLetterbox   = 1,
        AspectRatioStretch     = 2
    };

    AspectRatioMode GetAspectRatioMode() const throw();
    void SetAspectRatioMode(AspectRatioMode mode) throw();

    virtual ATL::CString GetTemplateType() const override;
    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CString    m_strMediaSocketId;
    ATL::CString    m_strEntranceAnimation;
    ATL::CString    m_strExitAnimation;
    DurationMode    m_durationMode;
    AspectRatioMode m_aspectRatioMode;
};

// ============================================================================
// EffectTemplateHandler
// ============================================================================
// Handles the mapping between effect types and their X3D templates.
// Resolves effect template references and provides template instances
// for rendering.
//
class STORYBOARD_API EffectTemplateHandler
{
public:
    EffectTemplateHandler();
    ~EffectTemplateHandler();

    // Registration
    HRESULT RegisterEffectTemplate(DWORD dwEffectType, EffectX3DTemplate* pTemplate);
    HRESULT UnregisterEffectTemplate(DWORD dwEffectType);

    // Lookup
    EffectX3DTemplate* FindTemplate(DWORD dwEffectType);
    const EffectX3DTemplate* FindTemplate(DWORD dwEffectType) const;

    // Default templates
    HRESULT LoadDefaultTemplates();
    bool HasDefaultTemplates() const throw();

    // Template creation
    EffectX3DTemplate* CreateTemplate(DWORD dwEffectType);
    void ReleaseTemplate(EffectX3DTemplate* pTemplate);

    // Count
    size_t GetTemplateCount() const throw();

private:
    struct EffectTemplateEntry
    {
        DWORD                   dwEffectType;
        EffectX3DTemplate*      pTemplate;
    };

    ATL::CAtlArray<EffectTemplateEntry> m_arrEntries;
    bool                                m_fDefaultsLoaded;
};

// ============================================================================
// ExtentSocketsHandler
// ============================================================================
// Manages the socket connections for extent rendering. Maps media items
// to template sockets and handles socket constraints during theme
// application.
//
class STORYBOARD_API ExtentSocketsHandler
{
public:
    ExtentSocketsHandler();
    ~ExtentSocketsHandler();

    // Initialize with a template's socket definitions
    HRESULT Initialize(BaseTemplate* pTemplate);

    // Map an extent to a socket
    HRESULT MapExtentToSocket(DWORD dwExtentId, LPCWSTR pszSocketId);
    HRESULT UnmapExtentFromSocket(DWORD dwExtentId);

    // Query mappings
    LPCWSTR FindSocketForExtent(DWORD dwExtentId) const;
    DWORD FindExtentForSocket(LPCWSTR pszSocketId) const;
    bool IsSocketOccupied(LPCWSTR pszSocketId) const throw();

    // Available socket count
    size_t GetAvailableSocketCount() const throw();
    size_t GetOccupiedSocketCount() const throw();
    size_t GetTotalSocketCount() const throw();

    // Reset all mappings
    void ResetMappings();

    // Validation
    HRESULT ValidateMappings() const;

private:
    struct ExtentSocketMapping
    {
        DWORD       dwExtentId;
        ATL::CString strSocketId;
    };

    BaseTemplate*                       m_pTemplate;
    ATL::CAtlArray<ExtentSocketMapping>  m_arrMappings;
};

} // namespace StoryboardManager

#endif // THEMES_TEMPLATES_H
