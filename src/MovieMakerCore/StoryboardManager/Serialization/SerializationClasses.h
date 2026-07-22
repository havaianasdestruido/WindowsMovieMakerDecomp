/*
 * SerializationClasses.h
 *
 * Stub RTTI class declarations for the StoryboardManager serialization/
 * binding framework. Provides placeholder implementations for ~100 RTTI
 * classes identified in the original Windows Live Movie Maker 2012 binary
 * but missing from the source recreation.
 *
 * Class families:
 *   - ModAttribute (typed attribute value wrappers)
 *   - BoundProperty (property binding system)
 *   - Serializable (serialization framework)
 *   - MediaItem serialization helpers
 *   - Template system classes
 *   - Extent range classes
 *   - Theme system classes
 *   - Property binding classes
 *   - Serialization context extensions
 *   - Miscellaneous helpers
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SERIALIZATIONCLASSES_H
#define SERIALIZATIONCLASSES_H

#include "SerializationContext.h"
#include "BoundPropertyDictionary.h"
#include "ValueVariantRef.h"

namespace StoryboardManager
{

// ============================================================================
// Forward declarations
// ============================================================================
class ModElementBase;
class ModBeginElement;
class MediaItemBase;
class MovieProject;
class MovieExtent;
class SerializationReader;
class SerializationWriter;

// ============================================================================
// ModAttribute classes
// ============================================================================
// Typed attribute value wrappers for serialization. Each ModAttribute*
// class wraps a typed value that can be read from / written to XML
// attributes during project file serialization.
//
// RTTI: ?AVModAttribute@@ ?AVModAttributeString@@ ?AVModAttributeInt@@
//       ?AVModAttributeBool@@ ?AVModAttributeFloat@@ ?AVModAttributeDouble@@
//       ?AVModAttributeGuid@@ ?AVModAttributeTime@@ ?AVModAttributeBlob@@
//       ?AVModAttributeRect@@ ?AVModAttributePoint@@ ?AVModAttributeSize@@
//       ?AVModAttributeColor@@ ?AVModAttributeVector@@ ?AVModAttributeMatrix@@
//
class STORYBOARD_API ModAttribute
{
public:
    ModAttribute();
    virtual ~ModAttribute();

    virtual HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName);
    virtual HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const;
    virtual bool IsDefault() const throw();

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

protected:
    ATL::CString m_strName;
};

class STORYBOARD_API ModAttributeString : public ModAttribute
{
public:
    ModAttributeString();
    ~ModAttributeString() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    ATL::CString GetValue() const;
    void SetValue(LPCWSTR pszValue);

private:
    ATL::CString m_strValue;
};

class STORYBOARD_API ModAttributeInt : public ModAttribute
{
public:
    ModAttributeInt();
    ~ModAttributeInt() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    int GetValue() const throw();
    void SetValue(int nValue) throw();

private:
    int m_nValue;
};

class STORYBOARD_API ModAttributeBool : public ModAttribute
{
public:
    ModAttributeBool();
    ~ModAttributeBool() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    bool GetValue() const throw();
    void SetValue(bool fValue) throw();

private:
    bool m_fValue;
};

class STORYBOARD_API ModAttributeFloat : public ModAttribute
{
public:
    ModAttributeFloat();
    ~ModAttributeFloat() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    float GetValue() const throw();
    void SetValue(float flValue) throw();

private:
    float m_flValue;
};

class STORYBOARD_API ModAttributeDouble : public ModAttribute
{
public:
    ModAttributeDouble();
    ~ModAttributeDouble() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    double GetValue() const throw();
    void SetValue(double dblValue) throw();

private:
    double m_dblValue;
};

class STORYBOARD_API ModAttributeGuid : public ModAttribute
{
public:
    ModAttributeGuid();
    ~ModAttributeGuid() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    GUID GetValue() const throw();
    void SetValue(const GUID& guid) throw();

private:
    GUID m_guid;
};

class STORYBOARD_API ModAttributeTime : public ModAttribute
{
public:
    ModAttributeTime();
    ~ModAttributeTime() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    LONGLONG GetValue() const throw();
    void SetValue(LONGLONG llTime) throw();

private:
    LONGLONG m_llTime;
};

class STORYBOARD_API ModAttributeBlob : public ModAttribute
{
public:
    ModAttributeBlob();
    ~ModAttributeBlob() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    const BYTE* GetData() const throw();
    DWORD GetSize() const throw();
    HRESULT SetData(const BYTE* pcbData, DWORD cbSize);

private:
    ATL::CAtlArray<BYTE> m_arrData;
};

class STORYBOARD_API ModAttributeRect : public ModAttribute
{
public:
    ModAttributeRect();
    ~ModAttributeRect() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    double GetX() const throw();
    double GetY() const throw();
    double GetWidth() const throw();
    double GetHeight() const throw();
    void SetRect(double dblX, double dblY, double dblWidth, double dblHeight) throw();

private:
    double m_dblX;
    double m_dblY;
    double m_dblWidth;
    double m_dblHeight;
};

class STORYBOARD_API ModAttributePoint : public ModAttribute
{
public:
    ModAttributePoint();
    ~ModAttributePoint() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    double GetX() const throw();
    double GetY() const throw();
    void SetPoint(double dblX, double dblY) throw();

private:
    double m_dblX;
    double m_dblY;
};

class STORYBOARD_API ModAttributeSize : public ModAttribute
{
public:
    ModAttributeSize();
    ~ModAttributeSize() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    double GetWidth() const throw();
    double GetHeight() const throw();
    void SetSize(double dblWidth, double dblHeight) throw();

private:
    double m_dblWidth;
    double m_dblHeight;
};

class STORYBOARD_API ModAttributeColor : public ModAttribute
{
public:
    ModAttributeColor();
    ~ModAttributeColor() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    DWORD GetColor() const throw();
    void SetColor(DWORD dwColor) throw();

private:
    DWORD m_dwColor;
};

class STORYBOARD_API ModAttributeVector : public ModAttribute
{
public:
    ModAttributeVector();
    ~ModAttributeVector() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    double GetX() const throw();
    double GetY() const throw();
    double GetZ() const throw();
    void SetVector(double dblX, double dblY, double dblZ) throw();

private:
    double m_dblX;
    double m_dblY;
    double m_dblZ;
};

class STORYBOARD_API ModAttributeMatrix : public ModAttribute
{
public:
    ModAttributeMatrix();
    ~ModAttributeMatrix() override;

    HRESULT ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName) override;
    HRESULT WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const override;
    bool IsDefault() const throw() override;

    const double* GetMatrix() const throw();
    void SetIdentity() throw();

private:
    double m_values[16];
};

// ============================================================================
// BoundProperty classes
// ============================================================================
// Property binding classes for X3D template property resolution. Each
// BoundProperty* class binds a named property to a specific value type
// for template-to-extent resolution.
//
// RTTI: ?AVBoundProperty@@ ?AVBoundPropertyString@@ ?AVBoundPropertyInt@@
//       ?AVBoundPropertyBool@@ ?AVBoundPropertyFloat@@ ?AVBoundPropertyDouble@@
//       ?AVBoundPropertyGuid@@ ?AVBoundPropertyTime@@ ?AVBoundPropertyBlob@@
//       ?AVBoundPropertyRef@@
//
class STORYBOARD_API BoundProperty
{
public:
    BoundProperty();
    virtual ~BoundProperty();

    virtual HRESULT Resolve(const BoundPropertyDictionary& dictionary);
    virtual HRESULT Serialize(ModBeginElement* pElement) const;
    virtual bool IsBound() const throw();

    ATL::CString GetPropertyName() const;
    void SetPropertyName(LPCWSTR pszName);

    ATL::CString GetTargetElement() const;
    void SetTargetElement(LPCWSTR pszElement);

protected:
    ATL::CString m_strPropertyName;
    ATL::CString m_strTargetElement;
    bool         m_fBound;
};

class STORYBOARD_API BoundPropertyString : public BoundProperty
{
public:
    BoundPropertyString();
    ~BoundPropertyString() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    ATL::CString GetStringValue() const;
    void SetStringValue(LPCWSTR pszValue);

private:
    ATL::CString m_strValue;
};

class STORYBOARD_API BoundPropertyInt : public BoundProperty
{
public:
    BoundPropertyInt();
    ~BoundPropertyInt() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    int GetIntValue() const throw();
    void SetIntValue(int nValue) throw();

private:
    int m_nValue;
};

class STORYBOARD_API BoundPropertyBool : public BoundProperty
{
public:
    BoundPropertyBool();
    ~BoundPropertyBool() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    bool GetBoolValue() const throw();
    void SetBoolValue(bool fValue) throw();

private:
    bool m_fValue;
};

class STORYBOARD_API BoundPropertyFloat : public BoundProperty
{
public:
    BoundPropertyFloat();
    ~BoundPropertyFloat() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    float GetFloatValue() const throw();
    void SetFloatValue(float flValue) throw();

private:
    float m_flValue;
};

class STORYBOARD_API BoundPropertyDouble : public BoundProperty
{
public:
    BoundPropertyDouble();
    ~BoundPropertyDouble() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    double GetDoubleValue() const throw();
    void SetDoubleValue(double dblValue) throw();

private:
    double m_dblValue;
};

class STORYBOARD_API BoundPropertyGuid : public BoundProperty
{
public:
    BoundPropertyGuid();
    ~BoundPropertyGuid() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    GUID GetGuidValue() const throw();
    void SetGuidValue(const GUID& guid) throw();

private:
    GUID m_guid;
};

class STORYBOARD_API BoundPropertyTime : public BoundProperty
{
public:
    BoundPropertyTime();
    ~BoundPropertyTime() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    LONGLONG GetTimeValue() const throw();
    void SetTimeValue(LONGLONG llTime) throw();

private:
    LONGLONG m_llTime;
};

class STORYBOARD_API BoundPropertyBlob : public BoundProperty
{
public:
    BoundPropertyBlob();
    ~BoundPropertyBlob() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    const BYTE* GetBlobData() const throw();
    DWORD GetBlobSize() const throw();

private:
    ATL::CAtlArray<BYTE> m_arrData;
};

class STORYBOARD_API BoundPropertyRef : public BoundProperty
{
public:
    BoundPropertyRef();
    ~BoundPropertyRef() override;

    HRESULT Resolve(const BoundPropertyDictionary& dictionary) override;
    HRESULT Serialize(ModBeginElement* pElement) const override;

    DWORD GetRefExtentId() const throw();
    void SetRefExtentId(DWORD dwExtentId) throw();

private:
    DWORD m_dwRefExtentId;
};

// ============================================================================
// Serializable classes
// ============================================================================
// Serialization framework classes providing the base infrastructure
// for project file read/write with versioning and factory support.
//
// RTTI: ?AVSerializable@@ ?AVSerializableBase@@ ?AVSerializablePartial@@
//       ?AVSerializablePartialImpl@@ ?AVSerializableContext@@
//       ?AVSerializableFactory@@ ?AVSerializableRegistry@@
//       ?AVSerializableFactoryMap@@ ?AVSerializableVersion@@
//       ?AVSerializableFlags@@
//
class STORYBOARD_API Serializable
{
public:
    Serializable();
    virtual ~Serializable();

    virtual HRESULT Serialize(SerializationWriter* pWriter) const;
    virtual HRESULT Deserialize(SerializationReader* pReader, SerializationContext& ctx);
    virtual DWORD GetSerializableVersion() const throw();
    virtual ATL::CString GetSerializableName() const;

protected:
    DWORD m_dwVersion;
};

class STORYBOARD_API SerializableBase : public Serializable
{
public:
    SerializableBase();
    ~SerializableBase() override;

    HRESULT Serialize(SerializationWriter* pWriter) const override;
    HRESULT Deserialize(SerializationReader* pReader, SerializationContext& ctx) override;
    DWORD GetSerializableVersion() const throw() override;
    ATL::CString GetSerializableName() const override;

    void SetVersion(DWORD dwVersion) throw();
};

class STORYBOARD_API SerializablePartial : public Serializable
{
public:
    SerializablePartial();
    ~SerializablePartial() override;

    HRESULT Serialize(SerializationWriter* pWriter) const override;
    HRESULT Deserialize(SerializationReader* pReader, SerializationContext& ctx) override;

    bool HasField(DWORD dwFieldId) const throw();
    void SetFieldPresent(DWORD dwFieldId, bool fPresent) throw();

private:
    DWORD m_dwFieldMask;
};

class STORYBOARD_API SerializablePartialImpl : public SerializablePartial
{
public:
    SerializablePartialImpl();
    ~SerializablePartialImpl() override;

    HRESULT Serialize(SerializationWriter* pWriter) const override;
    HRESULT Deserialize(SerializationReader* pReader, SerializationContext& ctx) override;

    void SetPartialData(DWORD dwFieldId, const ValueVariantRef& value);
    bool GetPartialData(DWORD dwFieldId, ValueVariantRef& value) const;

private:
    struct PartialEntry
    {
        DWORD          dwFieldId;
        ValueVariantRef value;
    };
    ATL::CAtlArray<PartialEntry> m_arrEntries;
};

class STORYBOARD_API SerializableContext
{
public:
    SerializableContext();
    ~SerializableContext();

    SerializationContext& GetContext();
    const SerializationContext& GetContext() const;

    void SetReader(SerializationReader* pReader);
    SerializationReader* GetReader() const;

    void SetWriter(SerializationWriter* pWriter);
    SerializationWriter* GetWriter() const;

private:
    SerializationContext  m_ctx;
    SerializationReader*  m_pReader;
    SerializationWriter*  m_pWriter;
};

class STORYBOARD_API SerializableFactory
{
public:
    SerializableFactory();
    virtual ~SerializableFactory();

    virtual Serializable* CreateInstance(LPCWSTR pszTypeName);
    virtual HRESULT RegisterType(LPCWSTR pszTypeName, DWORD dwVersion);
    virtual bool IsTypeRegistered(LPCWSTR pszTypeName) const;

protected:
    struct TypeEntry
    {
        ATL::CString strTypeName;
        DWORD        dwVersion;
    };
    ATL::CAtlArray<TypeEntry> m_arrTypes;
};

class STORYBOARD_API SerializableRegistry
{
public:
    SerializableRegistry();
    ~SerializableRegistry();

    static SerializableRegistry& GetInstance();

    HRESULT RegisterFactory(LPCWSTR pszNamespace, SerializableFactory* pFactory);
    SerializableFactory* FindFactory(LPCWSTR pszNamespace) const;
    void UnregisterAll();

private:
    struct RegistryEntry
    {
        ATL::CString       strNamespace;
        SerializableFactory* pFactory;
    };
    ATL::CAtlArray<RegistryEntry> m_arrEntries;
};

class STORYBOARD_API SerializableFactoryMap
{
public:
    SerializableFactoryMap();
    ~SerializableFactoryMap();

    HRESULT AddMapping(LPCWSTR pszTypeName, SerializableFactory* pFactory);
    SerializableFactory* FindMapping(LPCWSTR pszTypeName) const;
    size_t GetCount() const throw();

private:
    struct MappingEntry
    {
        ATL::CString       strTypeName;
        SerializableFactory* pFactory;
    };
    ATL::CAtlArray<MappingEntry> m_arrMappings;
};

class STORYBOARD_API SerializableVersion
{
public:
    SerializableVersion();
    SerializableVersion(DWORD dwMajor, DWORD dwMinor);
    ~SerializableVersion();

    DWORD GetMajor() const throw();
    DWORD GetMinor() const throw();
    void SetMajor(DWORD dwMajor) throw();
    void SetMinor(DWORD dwMinor) throw();

    bool operator>=(const SerializableVersion& other) const;
    bool operator<(const SerializableVersion& other) const;

private:
    DWORD m_dwMajor;
    DWORD m_dwMinor;
};

class STORYBOARD_API SerializableFlags
{
public:
    SerializableFlags();
    ~SerializableFlags();

    DWORD GetFlags() const throw();
    void SetFlags(DWORD dwFlags) throw();
    void AddFlag(DWORD dwFlag) throw();
    void RemoveFlag(DWORD dwFlag) throw();
    bool HasFlag(DWORD dwFlag) const throw();

private:
    DWORD m_dwFlags;
};

// ============================================================================
// MediaItem serialization helper classes
// ============================================================================
// Serialization support classes for the MediaItem hierarchy. Provide
// element-level read/write for media item properties.
//
// RTTI: ?AVMediaItemSerializer@@ ?AVMediaItemVideoSerializer@@
//       ?AVMediaItemAudioSerializer@@ ?AVMediaItemPhotoSerializer@@
//       ?AVMediaItemTransitionSerializer@@ ?AVMediaItemEffectSerializer@@
//       ?AVMediaItemTextSerializer@@ ?AVMediaItemGroupSerializer@@
//
class STORYBOARD_API MediaItemSerializer
{
public:
    MediaItemSerializer();
    virtual ~MediaItemSerializer();

    virtual HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter);
    virtual HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                                    SerializationContext& ctx);
    virtual LPCWSTR GetElementName() const;
};

class STORYBOARD_API MediaItemVideoSerializer : public MediaItemSerializer
{
public:
    MediaItemVideoSerializer();
    ~MediaItemVideoSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemAudioSerializer : public MediaItemSerializer
{
public:
    MediaItemAudioSerializer();
    ~MediaItemAudioSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemPhotoSerializer : public MediaItemSerializer
{
public:
    MediaItemPhotoSerializer();
    ~MediaItemPhotoSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemTransitionSerializer : public MediaItemSerializer
{
public:
    MediaItemTransitionSerializer();
    ~MediaItemTransitionSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemEffectSerializer : public MediaItemSerializer
{
public:
    MediaItemEffectSerializer();
    ~MediaItemEffectSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemTextSerializer : public MediaItemSerializer
{
public:
    MediaItemTextSerializer();
    ~MediaItemTextSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

class STORYBOARD_API MediaItemGroupSerializer : public MediaItemSerializer
{
public:
    MediaItemGroupSerializer();
    ~MediaItemGroupSerializer() override;

    HRESULT SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter) override;
    HRESULT DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                            SerializationContext& ctx) override;
    LPCWSTR GetElementName() const override;
};

// ============================================================================
// Template system classes
// ============================================================================
// Template loading, parsing, and caching classes for the X3D template
// system used by the theme engine.
//
// RTTI: ?AVTemplateTable@@ ?AVTemplateEntry@@ ?AVTemplateCategory@@
//       ?AVTemplateRegistry@@ ?AVTemplateLoader@@ ?AVTemplateParser@@
//       ?AVTemplateValidator@@ ?AVTemplateCache@@
//


class STORYBOARD_API TemplateCategory
{
public:
    TemplateCategory();
    ~TemplateCategory();

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    size_t GetEntryCount() const throw();
    TemplateEntry* GetEntryAt(size_t nIndex);
    const TemplateEntry* GetEntryAt(size_t nIndex) const;

    void AddEntry(TemplateEntry* pEntry);
    void RemoveAll();

private:
    ATL::CString m_strName;
    ATL::CAtlArray<TemplateEntry*> m_arrEntries;
};



class STORYBOARD_API TemplateRegistry
{
public:
    TemplateRegistry();
    ~TemplateRegistry();

    static TemplateRegistry& GetInstance();

    HRESULT RegisterTable(LPCWSTR pszThemeName, TemplateTable* pTable);
    TemplateTable* FindTable(LPCWSTR pszThemeName) const;
    void UnregisterAll();

private:
    struct RegistryEntry
    {
        ATL::CString  strThemeName;
        TemplateTable* pTable;
    };
    ATL::CAtlArray<RegistryEntry> m_arrEntries;
};

class STORYBOARD_API TemplateLoader
{
public:
    TemplateLoader();
    ~TemplateLoader();

    HRESULT LoadFromFile(LPCWSTR pszFilePath, TemplateTable* pTable);
    HRESULT LoadFromStream(IStream* pStream, TemplateTable* pTable);
    HRESULT LoadFromResource(HMODULE hModule, LPCWSTR pszResourceName, TemplateTable* pTable);

    ATL::CString GetLastErrorMessage() const;

private:
    ATL::CString m_strLastError;
};

class STORYBOARD_API TemplateParser
{
public:
    TemplateParser();
    ~TemplateParser();

    HRESULT ParseElement(IXmlReader* pReader, TemplateTable* pTable);
    HRESULT ParseEntry(IXmlReader* pReader, TemplateCategory* pCategory);

    ATL::CString GetLastErrorMessage() const;

private:
    ATL::CString m_strLastError;
};

class STORYBOARD_API TemplateValidator
{
public:
    TemplateValidator();
    ~TemplateValidator();

    bool ValidateEntry(const TemplateEntry& entry) const;
    bool ValidateCategory(const TemplateCategory& category) const;
    bool ValidateTable(const TemplateTable& table) const;

    ATL::CString GetValidationErrors() const;

private:
    ATL::CString m_strErrors;
};

class STORYBOARD_API TemplateCache
{
public:
    TemplateCache();
    ~TemplateCache();

    HRESULT CacheTemplate(LPCWSTR pszKey, TemplateTable* pTable);
    TemplateTable* GetCachedTemplate(LPCWSTR pszKey) const;
    bool HasCachedTemplate(LPCWSTR pszKey) const;
    void Evict(LPCWSTR pszKey);
    void Clear();

    size_t GetCacheSize() const throw();

private:
    struct CacheEntry
    {
        ATL::CString  strKey;
        TemplateTable* pTable;
    };
    ATL::CAtlArray<CacheEntry> m_arrEntries;
};

// ============================================================================
// Extent range classes
// ============================================================================
// Range classes for defining extents on the timeline within specific
// tracks (video, audio, text, effects, transitions).
//
// RTTI: ?AVExtentBase@@ ?AVExtentDuration@@ ?AVExtentTimeRange@@
//       ?AVExtentVideoRange@@ ?AVExtentAudioRange@@ ?AVExtentTextRange@@
//       ?AVExtentEffectRange@@ ?AVExtentTransitionRange@@
//
class STORYBOARD_API ExtentBase
{
public:
    ExtentBase();
    virtual ~ExtentBase();

    DWORD GetExtentId() const throw();
    void SetExtentId(DWORD dwId) throw();

    virtual LONGLONG GetStartHns() const throw();
    virtual LONGLONG GetEndHns() const throw();
    virtual LONGLONG GetDurationHns() const throw();

    virtual bool IsValid() const throw();

protected:
    DWORD   m_dwExtentId;
    LONGLONG m_llStartHns;
    LONGLONG m_llEndHns;
};

class STORYBOARD_API ExtentDuration : public ExtentBase
{
public:
    ExtentDuration();
    ~ExtentDuration() override;

    LONGLONG GetDurationHns() const throw() override;
    void SetDurationHns(LONGLONG llDuration) throw();

    bool IsValid() const throw() override;

private:
    LONGLONG m_llDurationHns;
};

class STORYBOARD_API ExtentTimeRange : public ExtentBase
{
public:
    ExtentTimeRange();
    ~ExtentTimeRange() override;

    LONGLONG GetStartHns() const throw() override;
    LONGLONG GetEndHns() const throw() override;
    LONGLONG GetDurationHns() const throw() override;

    void SetRange(LONGLONG llStart, LONGLONG llEnd) throw();
    bool Contains(LONGLONG llTime) const throw();

    bool IsValid() const throw() override;
};

class STORYBOARD_API ExtentVideoRange : public ExtentBase
{
public:
    ExtentVideoRange();
    ~ExtentVideoRange() override;

    DWORD GetVideoStreamIndex() const throw();
    void SetVideoStreamIndex(DWORD dwIndex) throw();

    bool HasMotionEffect() const throw();
    void SetHasMotionEffect(bool fHas) throw();

    bool IsValid() const throw() override;

private:
    DWORD m_dwVideoStreamIndex;
    bool  m_fHasMotionEffect;
};

class STORYBOARD_API ExtentAudioRange : public ExtentBase
{
public:
    ExtentAudioRange();
    ~ExtentAudioRange() override;

    DWORD GetAudioStreamIndex() const throw();
    void SetAudioStreamIndex(DWORD dwIndex) throw();

    double GetVolume() const throw();
    void SetVolume(double dblVolume) throw();

    bool IsValid() const throw() override;

private:
    DWORD  m_dwAudioStreamIndex;
    double m_dblVolume;
};

class STORYBOARD_API ExtentTextRange : public ExtentBase
{
public:
    ExtentTextRange();
    ~ExtentTextRange() override;

    ATL::CString GetText() const;
    void SetText(LPCWSTR pszText);

    DWORD GetAnimationId() const throw();
    void SetAnimationId(DWORD dwAnimId) throw();

    bool IsValid() const throw() override;

private:
    ATL::CString m_strText;
    DWORD        m_dwAnimationId;
};

class STORYBOARD_API ExtentEffectRange : public ExtentBase
{
public:
    ExtentEffectRange();
    ~ExtentEffectRange() override;

    DWORD GetEffectId() const throw();
    void SetEffectId(DWORD dwId) throw();

    double GetIntensity() const throw();
    void SetIntensity(double dblIntensity) throw();

    bool IsValid() const throw() override;

private:
    DWORD  m_dwEffectId;
    double m_dblIntensity;
};

class STORYBOARD_API ExtentTransitionRange : public ExtentBase
{
public:
    ExtentTransitionRange();
    ~ExtentTransitionRange() override;

    DWORD GetTransitionId() const throw();
    void SetTransitionId(DWORD dwId) throw();

    double GetOverlapDuration() const throw();
    void SetOverlapDuration(double dblOverlap) throw();

    bool IsValid() const throw() override;

private:
    DWORD  m_dwTransitionId;
    double m_dblOverlapDuration;
};

// ============================================================================
// Theme system classes
// ============================================================================
// Theme property classes for the design theme engine. Each Theme*
// class represents a specific design property (color, font, effect, etc.)
// that can be applied across the project.
//
// RTTI: ?AVThemeBase@@ ?AVThemeColor@@ ?AVThemeFont@@ ?AVThemeEffect@@
//       ?AVThemeTransition@@ ?AVThemeStyle@@ ?AVThemeTemplate@@
//       ?AVThemeRegistry@@
//
class STORYBOARD_API ThemeBase
{
public:
    ThemeBase();
    virtual ~ThemeBase();

    virtual HRESULT Apply(MovieProject* pProject);
    virtual HRESULT Remove(MovieProject* pProject);
    virtual bool IsValid() const throw();

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    DWORD GetThemeId() const throw();
    void SetThemeId(DWORD dwId) throw();

protected:
    ATL::CString m_strName;
    DWORD        m_dwThemeId;
};

class STORYBOARD_API ThemeColor : public ThemeBase
{
public:
    ThemeColor();
    ~ThemeColor() override;

    HRESULT Apply(MovieProject* pProject) override;
    HRESULT Remove(MovieProject* pProject) override;

    DWORD GetPrimaryColor() const throw();
    void SetPrimaryColor(DWORD dwColor) throw();

    DWORD GetSecondaryColor() const throw();
    void SetSecondaryColor(DWORD dwColor) throw();

    DWORD GetAccentColor() const throw();
    void SetAccentColor(DWORD dwColor) throw();

    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

private:
    DWORD m_dwPrimaryColor;
    DWORD m_dwSecondaryColor;
    DWORD m_dwAccentColor;
    DWORD m_dwBackgroundColor;
};

class STORYBOARD_API ThemeFont : public ThemeBase
{
public:
    ThemeFont();
    ~ThemeFont() override;

    HRESULT Apply(MovieProject* pProject) override;
    HRESULT Remove(MovieProject* pProject) override;

    ATL::CString GetTitleFontFamily() const;
    void SetTitleFontFamily(LPCWSTR pszFont);

    ATL::CString GetBodyFontFamily() const;
    void SetBodyFontFamily(LPCWSTR pszFont);

    float GetTitleFontSize() const throw();
    void SetTitleFontSize(float flSize) throw();

    float GetBodyFontSize() const throw();
    void SetBodyFontSize(float flSize) throw();

private:
    ATL::CString m_strTitleFont;
    ATL::CString m_strBodyFont;
    float        m_flTitleFontSize;
    float        m_flBodyFontSize;
};

class STORYBOARD_API ThemeEffectProp : public ThemeBase
{
public:
    ThemeEffectProp();
    ~ThemeEffectProp() override;

    HRESULT Apply(MovieProject* pProject) override;
    HRESULT Remove(MovieProject* pProject) override;

    DWORD GetEffectId() const throw();
    void SetEffectId(DWORD dwId) throw();

    double GetIntensity() const throw();
    void SetIntensity(double dblIntensity) throw();

private:
    DWORD  m_dwEffectId;
    double m_dblIntensity;
};



class STORYBOARD_API ThemeStyle : public ThemeBase
{
public:
    ThemeStyle();
    ~ThemeStyle() override;

    HRESULT Apply(MovieProject* pProject) override;
    HRESULT Remove(MovieProject* pProject) override;

    ATL::CString GetStyleName() const;
    void SetStyleName(LPCWSTR pszStyle);

    BoundPropertyDictionary& GetProperties();
    const BoundPropertyDictionary& GetProperties() const;

private:
    ATL::CString m_strStyleName;
    BoundPropertyDictionary m_properties;
};

class STORYBOARD_API ThemeTemplate : public ThemeBase
{
public:
    ThemeTemplate();
    ~ThemeTemplate() override;

    HRESULT Apply(MovieProject* pProject) override;
    HRESULT Remove(MovieProject* pProject) override;

    TemplateTable* GetTemplateTable();
    void SetTemplateTable(TemplateTable* pTable);

    bool HasTemplateTable() const throw();

private:
    TemplateTable* m_pTemplateTable;
};

class STORYBOARD_API ThemeRegistry
{
public:
    ThemeRegistry();
    ~ThemeRegistry();

    static ThemeRegistry& GetInstance();

    HRESULT RegisterTheme(LPCWSTR pszName, ThemeBase* pTheme);
    ThemeBase* FindTheme(LPCWSTR pszName) const;
    size_t GetThemeCount() const throw();
    ThemeBase* GetThemeAt(size_t nIndex);
    void UnregisterAll();

private:
    struct RegistryEntry
    {
        ATL::CString strName;
        ThemeBase*   pTheme;
    };
    ATL::CAtlArray<RegistryEntry> m_arrEntries;
};

// ============================================================================
// Property binding classes
// ============================================================================
// Property binding infrastructure for connecting X3D template properties
// to serialized extent values.
//
// RTTI: ?AVPropertyBinding@@ ?AVPropertyBindingSource@@
//       ?AVPropertyBindingTarget@@ ?AVPropertyBindingConverter@@
//       ?AVPropertyBindingValidator@@ ?AVPropertyBindingManager@@
//
class STORYBOARD_API PropertyBinding
{
public:
    PropertyBinding();
    virtual ~PropertyBinding();

    virtual HRESULT Bind();
    virtual HRESULT Unbind();
    virtual bool IsBound() const throw();

    ATL::CString GetSourceProperty() const;
    void SetSourceProperty(LPCWSTR pszProperty);

    ATL::CString GetTargetProperty() const;
    void SetTargetProperty(LPCWSTR pszProperty);

protected:
    ATL::CString m_strSourceProperty;
    ATL::CString m_strTargetProperty;
    bool         m_fBound;
};

class STORYBOARD_API PropertyBindingSource : public PropertyBinding
{
public:
    PropertyBindingSource();
    ~PropertyBindingSource() override;

    HRESULT Bind() override;
    HRESULT Unbind() override;

    HRESULT SetPropertyValue(const ValueVariantRef& value);
    HRESULT GetPropertyValue(ValueVariantRef& value) const;

    DWORD GetSourceId() const throw();
    void SetSourceId(DWORD dwId) throw();

private:
    DWORD          m_dwSourceId;
    ValueVariantRef m_currentValue;
};

class STORYBOARD_API PropertyBindingTarget : public PropertyBinding
{
public:
    PropertyBindingTarget();
    ~PropertyBindingTarget() override;

    HRESULT Bind() override;
    HRESULT Unbind() override;

    HRESULT ApplyValue(const ValueVariantRef& value);
    HRESULT ReadValue(ValueVariantRef& value) const;

    DWORD GetTargetExtentId() const throw();
    void SetTargetExtentId(DWORD dwId) throw();

private:
    DWORD          m_dwTargetExtentId;
    ValueVariantRef m_currentValue;
};

class STORYBOARD_API PropertyBindingConverter
{
public:
    PropertyBindingConverter();
    virtual ~PropertyBindingConverter();

    virtual HRESULT Convert(const ValueVariantRef& source, ValueVariantRef& target);
    virtual bool CanConvert(ValueVariantRef::VariantType sourceType, ValueVariantRef::VariantType targetType) const throw();

    static PropertyBindingConverter* GetDefaultConverter();
};

class STORYBOARD_API PropertyBindingValidator
{
public:
    PropertyBindingValidator();
    virtual ~PropertyBindingValidator();

    virtual bool Validate(const ValueVariantRef& value);
    virtual HRESULT GetLastValidationError(ATL::CString& strError);

    static PropertyBindingValidator* GetDefaultValidator();

private:
    ATL::CString m_strLastError;
};

class STORYBOARD_API PropertyBindingManager
{
public:
    PropertyBindingManager();
    ~PropertyBindingManager();

    HRESULT AddBinding(PropertyBinding* pBinding);
    HRESULT RemoveBinding(LPCWSTR pszSourceProperty, LPCWSTR pszTargetProperty);

    HRESULT ResolveAll(const BoundPropertyDictionary& dictionary);
    HRESULT ApplyAll();

    size_t GetBindingCount() const throw();
    PropertyBinding* GetBindingAt(size_t nIndex);
    const PropertyBinding* GetBindingAt(size_t nIndex) const;

    void RemoveAll();

private:
    ATL::CAtlArray<PropertyBinding*> m_arrBindings;
};

// ============================================================================
// Serialization context extension classes
// ============================================================================
// Extended serialization context classes providing versioned read/write
// context and registry for the serialization pipeline.
//
// RTTI: ?AVSerializationContextBase@@ ?AVSerializationContextRead@@
//       ?AVSerializationContextWrite@@ ?AVSerializationContextVersion@@
//       ?AVSerializationContextRegistry@@
//
class STORYBOARD_API SerializationAttributeList
{
public:
    SerializationAttributeList();
    ~SerializationAttributeList();

    HRESULT AddAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    HRESULT GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const;
    bool HasAttribute(LPCWSTR pszName) const;

    size_t GetCount() const throw();
    LPCWSTR GetNameAt(size_t nIndex) const;
    LPCWSTR GetValueAt(size_t nIndex) const;

    void RemoveAll();

private:
    struct AttrEntry
    {
        ATL::CString strName;
        ATL::CString strValue;
    };
    ATL::CAtlArray<AttrEntry> m_arrEntries;
};

class STORYBOARD_API SerializationContextBase
{
public:
    SerializationContextBase();
    virtual ~SerializationContextBase();

    virtual HRESULT Initialize();
    virtual void Shutdown();

    SerializationContext& GetContext();
    const SerializationContext& GetContext() const;

    HRESULT GetLastResult() const throw();

protected:
    SerializationContext m_ctx;
    HRESULT             m_hrLast;
};

class STORYBOARD_API SerializationContextRead : public SerializationContextBase
{
public:
    SerializationContextRead();
    ~SerializationContextRead() override;

    HRESULT Initialize() override;

    void SetReader(SerializationReader* pReader);
    SerializationReader* GetReader() const;

    HRESULT BeginElement(LPCWSTR pszName);
    HRESULT EndElement(LPCWSTR pszName);
    HRESULT ReadAttribute(LPCWSTR pszName, ATL::CString& strValue);

    HRESULT CacheAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    void ClearAttributes();

private:
    SerializationReader* m_pReader;
    SerializationAttributeList m_attributes;
};

class STORYBOARD_API SerializationContextWrite : public SerializationContextBase
{
public:
    SerializationContextWrite();
    ~SerializationContextWrite() override;

    HRESULT Initialize() override;

    void SetWriter(SerializationWriter* pWriter);
    SerializationWriter* GetWriter() const;

    HRESULT BeginElement(LPCWSTR pszName);
    HRESULT EndElement();
    HRESULT WriteAttribute(LPCWSTR pszName, LPCWSTR pszValue);

private:
    SerializationWriter* m_pWriter;
};

class STORYBOARD_API SerializationContextVersion
{
public:
    SerializationContextVersion();
    ~SerializationContextVersion();

    DWORD GetMajorVersion() const throw();
    DWORD GetMinorVersion() const throw();
    void SetVersion(DWORD dwMajor, DWORD dwMinor) throw();

    bool IsVersionSupported(DWORD dwMinMajor, DWORD dwMinMajorMinor,
                            DWORD dwMaxMajor, DWORD dwMaxMinor) const;

    static SerializationContextVersion Current();

private:
    DWORD m_dwMajor;
    DWORD m_dwMinor;
};

class STORYBOARD_API SerializationContextRegistry
{
public:
    SerializationContextRegistry();
    ~SerializationContextRegistry();

    static SerializationContextRegistry& GetInstance();

    HRESULT RegisterContext(LPCWSTR pszName, SerializationContextBase* pContext);
    SerializationContextBase* FindContext(LPCWSTR pszName) const;
    void UnregisterAll();

private:
    struct RegistryEntry
    {
        ATL::CString            strName;
        SerializationContextBase* pContext;
    };
    ATL::CAtlArray<RegistryEntry> m_arrEntries;
};

// ============================================================================
// Miscellaneous helper classes
// ============================================================================
// Utility classes found in RTTI analysis of the serialization system.

class STORYBOARD_API SerializationElementMap
{
public:
    SerializationElementMap();
    ~SerializationElementMap();

    HRESULT AddMapping(LPCWSTR pszElementName, DWORD dwHandlerId);
    int FindMapping(LPCWSTR pszElementName) const;
    size_t GetCount() const throw();

    ATL::CString GetElementNameAt(size_t nIndex) const;
    DWORD GetHandlerIdAt(size_t nIndex) const;

private:
    struct MapEntry
    {
        ATL::CString strElementName;
        DWORD        dwHandlerId;
    };
    ATL::CAtlArray<MapEntry> m_arrEntries;
};

class STORYBOARD_API SerializationElementState
{
public:
    SerializationElementState();
    ~SerializationElementState();

    ATL::CString GetElementName() const;
    void SetElementName(LPCWSTR pszName);

    DWORD GetDepth() const throw();
    void SetDepth(DWORD dwDepth) throw();

    bool IsComplete() const throw();
    void SetComplete(bool fComplete) throw();

    SerializationAttributeList& GetAttributes();
    const SerializationAttributeList& GetAttributes() const;

private:
    ATL::CString                m_strElementName;
    DWORD                       m_dwDepth;
    bool                        m_fComplete;
    SerializationAttributeList  m_attributes;
};

class STORYBOARD_API SerializationErrorInfo
{
public:
    SerializationErrorInfo();
    ~SerializationErrorInfo();

    HRESULT GetHResult() const throw();
    void SetHResult(HRESULT hr) throw();

    ATL::CString GetErrorMessage() const;
    void SetErrorMessage(LPCWSTR pszMessage);

    ATL::CString GetSourceElement() const;
    void SetSourceElement(LPCWSTR pszElement);

    DWORD GetLineNumber() const throw();
    void SetLineNumber(DWORD dwLine) throw();

    bool HasError() const throw();
    void Clear();

private:
    HRESULT     m_hr;
    ATL::CString m_strMessage;
    ATL::CString m_strSourceElement;
    DWORD       m_dwLineNumber;
};

class STORYBOARD_API SerializationBookmark
{
public:
    SerializationBookmark();
    ~SerializationBookmark();

    HRESULT SavePosition(IXmlReader* pReader);
    HRESULT RestorePosition(IXmlReader* pReader);

    bool HasPosition() const throw();
    void Clear();

private:
    DWORD m_dwElementDepth;
    DWORD m_dwAttributeIndex;
};

class STORYBOARD_API SerializationNamespaceManager
{
public:
    SerializationNamespaceManager();
    ~SerializationNamespaceManager();

    HRESULT RegisterNamespace(LPCWSTR pszPrefix, LPCWSTR pszUri);
    HRESULT LookupNamespace(LPCWSTR pszPrefix, ATL::CString& strUri) const;
    HRESULT LookupPrefix(LPCWSTR pszUri, ATL::CString& strPrefix) const;

    size_t GetCount() const throw();
    void Clear();

private:
    struct NamespaceEntry
    {
        ATL::CString strPrefix;
        ATL::CString strUri;
    };
    ATL::CAtlArray<NamespaceEntry> m_arrEntries;
};

class STORYBOARD_API SerializationElementStack
{
public:
    SerializationElementStack();
    ~SerializationElementStack();

    void PushElement(LPCWSTR pszName, DWORD dwDepth);
    bool PopElement(LPCWSTR pszName);
    bool PeekElement(ATL::CString& strName) const;

    DWORD GetDepth() const throw();
    bool IsEmpty() const throw();
    void Clear();

private:
    struct StackEntry
    {
        ATL::CString strName;
        DWORD        dwDepth;
    };
    ATL::CAtlArray<StackEntry> m_arrEntries;
};

class STORYBOARD_API SerializationBuffer
{
public:
    SerializationBuffer();
    ~SerializationBuffer();

    HRESULT Reserve(DWORD cbSize);
    HRESULT Append(const BYTE* pcbData, DWORD cbSize);
    HRESULT Clear();

    const BYTE* GetBuffer() const throw();
    DWORD GetSize() const throw();

    HRESULT WriteToFile(LPCWSTR pszFilePath) const;
    HRESULT ReadFromFile(LPCWSTR pszFilePath);

private:
    ATL::CAtlArray<BYTE> m_arrBuffer;
    DWORD                m_cbSize;
};

class STORYBOARD_API SerializationTextEncoder
{
public:
    SerializationTextEncoder();
    ~SerializationTextEncoder();

    static HRESULT EncodeXmlString(LPCWSTR pszInput, ATL::CString& strOutput);
    static HRESULT DecodeXmlString(LPCWSTR pszInput, ATL::CString& strOutput);

    static HRESULT EncodeBase64(const BYTE* pcbData, DWORD cbData, ATL::CString& strOutput);
    static HRESULT DecodeBase64(LPCWSTR pszInput, ATL::CAtlArray<BYTE>& arrOutput);
};

} // namespace StoryboardManager

#endif // SERIALIZATIONCLASSES_H
