/*
 * ComFactory.h
 *
 * ATL COM class factory registration for key StoryboardManager objects.
 * Provides CComCoClass-based COM wrappers that enable CoCreateInstance
 * creation of StoryboardManager, MovieProject, TimelineTrack, and
 * SerializationWriter objects.
 *
 * Each wrapper class uses ATL's CComObjectRootEx + CComCoClass pattern
 * consistent with the existing Sundance behavior classes.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef MOVIECORE_COM_FACTORY_H
#define MOVIECORE_COM_FACTORY_H

#include "pch.h"
#include "StoryboardManager/StoryboardManager.h"
#include "StoryboardManager/MovieProject.h"
#include "StoryboardManager/TimelineTrack.h"
#include "StoryboardManager/Serialization/SerializationWriter.h"

// ============================================================================
// CLSIDs for core COM objects
// Generated deterministically from type names via GUIDGEN-style constants.
// ============================================================================

// {A1B2C3D4-E5F6-4829-9B01-234567890ABC}
DEFINE_GUID(CLSID_StoryboardManagerObj,
    0xa1b2c3d4, 0xe5f6, 0x4829, 0x9b, 0x01, 0x23, 0x45, 0x67, 0x89, 0x0a, 0xbc);

// {B2C3D4E5-F6A7-4930-A112-34567890ABCD}
DEFINE_GUID(CLSID_MovieProjectObj,
    0xb2c3d4e5, 0xf6a7, 0x4930, 0xa1, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd);

// {C3D4E5F6-A7B8-4041-B223-4567890ABCDE}
DEFINE_GUID(CLSID_TimelineTrackObj,
    0xc3d4e5f6, 0xa7b8, 0x4041, 0xb2, 0x23, 0x45, 0x67, 0x89, 0x0a, 0xbc, 0xde);

// {D4E5F6A7-B8C9-4152-C334-567890ABCDEF}
DEFINE_GUID(CLSID_SerializationWriterObj,
    0xd4e5f6a7, 0xb8c9, 0x4152, 0xc3, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef);

// ============================================================================
// Registry resource IDs for COM class registration
// ============================================================================
#define IDR_STORYBOARD_MANAGER_OBJ     5001
#define IDR_MOVIE_PROJECT_OBJ          5002
#define IDR_TIMELINE_TRACK_OBJ         5003
#define IDR_SERIALIZATION_WRITER_OBJ   5004

// ============================================================================
// CComStoryboardManager
// ============================================================================
// ATL COM wrapper for StoryboardManager::StoryboardManager.
// Creates and owns an inner StoryboardManager instance.
//
class ATL_NO_VTABLE CComStoryboardManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComStoryboardManager, &CLSID_StoryboardManagerObj>
{
public:
    CComStoryboardManager()
        : m_pInner(NULL)
    {
    }

    ~CComStoryboardManager()
    {
        delete m_pInner;
        m_pInner = NULL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_STORYBOARD_MANAGER_OBJ)
    DECLARE_NOT_AGGREGATABLE(CComStoryboardManager)

    // Manual COM map (avoids ATL CComQIPtr<IUnknown> template conflict in MSVC 14.44)
    BEGIN_COM_MAP(CComStoryboardManager)
        COM_INTERFACE_ENTRY_IID(IID_IUnknown, CComStoryboardManager)
    END_COM_MAP()

    // IStoryboardManager -- exposed methods
    HRESULT STDMETHODCALLTYPE Initialize()
    {
        return StoryboardManager::StoryboardManagerInitialize();
    }

    void STDMETHODCALLTYPE Shutdown()
    {
        StoryboardManager::StoryboardManagerShutdown();
    }

    StoryboardManager::StoryboardManager* GetInner()
    {
        if (!m_pInner)
            m_pInner = new StoryboardManager::StoryboardManager();
        return m_pInner;
    }

private:
    StoryboardManager::StoryboardManager* m_pInner;
};

// ============================================================================
// CComMovieProject
// ============================================================================
// ATL COM wrapper for StoryboardManager::MovieProject.
// Creates a new MovieProject via the CreateEmpty() factory method.
//
class ATL_NO_VTABLE CComMovieProject :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComMovieProject, &CLSID_MovieProjectObj>
{
public:
    CComMovieProject()
        : m_pInner(NULL)
    {
    }

    ~CComMovieProject()
    {
        delete m_pInner;
        m_pInner = NULL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_MOVIE_PROJECT_OBJ)
    DECLARE_NOT_AGGREGATABLE(CComMovieProject)

    // Manual COM map (avoids ATL CComQIPtr<IUnknown> template conflict in MSVC 14.44)
    BEGIN_COM_MAP(CComMovieProject)
        COM_INTERFACE_ENTRY_IID(IID_IUnknown, CComMovieProject)
    END_COM_MAP()

    StoryboardManager::MovieProject* GetInner()
    {
        if (!m_pInner)
            m_pInner = StoryboardManager::MovieProject::CreateEmpty();
        return m_pInner;
    }

    HRESULT STDMETHODCALLTYPE Save(LPCWSTR pszPath)
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->Save(pszPath);
    }

    HRESULT STDMETHODCALLTYPE Load(LPCWSTR pszPath)
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->Load(pszPath);
    }

    HRESULT STDMETHODCALLTYPE SaveToStream(IStream* pStream)
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->SaveToStream(pStream);
    }

    HRESULT STDMETHODCALLTYPE LoadFromStream(IStream* pStream)
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->LoadFromStream(pStream);
    }

    LPCWSTR STDMETHODCALLTYPE GetProjectPath()
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return L"";
        static ATL::CString sPath;
        sPath = p->GetProjectPath();
        return sPath.GetString();
    }

    HRESULT STDMETHODCALLTYPE ImportMedia(LPCWSTR pszFilePath, StoryboardManager::TimelineTrackType trackType)
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->ImportMedia(pszFilePath, trackType);
    }

    size_t STDMETHODCALLTYPE GetMediaItemCount()
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return 0;
        return p->GetMediaItemCount();
    }

    size_t STDMETHODCALLTYPE GetTotalExtentCount()
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return 0;
        return p->GetTotalExtentCount();
    }

    HRESULT STDMETHODCALLTYPE Validate()
    {
        StoryboardManager::MovieProject* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->Validate();
    }

private:
    StoryboardManager::MovieProject* m_pInner;
};

// ============================================================================
// CComTimelineTrack
// ============================================================================
// ATL COM wrapper for StoryboardManager::TimelineTrack.
// Creates a TimelineTrack with the specified type.
//
class ATL_NO_VTABLE CComTimelineTrack :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComTimelineTrack, &CLSID_TimelineTrackObj>
{
public:
    CComTimelineTrack()
        : m_pInner(NULL)
    {
    }

    ~CComTimelineTrack()
    {
        delete m_pInner;
        m_pInner = NULL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_TRACK_OBJ)
    DECLARE_NOT_AGGREGATABLE(CComTimelineTrack)

    // Manual COM map (avoids ATL CComQIPtr<IUnknown> template conflict in MSVC 14.44)
    BEGIN_COM_MAP(CComTimelineTrack)
        COM_INTERFACE_ENTRY_IID(IID_IUnknown, CComTimelineTrack)
    END_COM_MAP()

    HRESULT STDMETHODCALLTYPE Initialize(StoryboardManager::TimelineTrackType type)
    {
        if (m_pInner) return E_UNEXPECTED;
        m_pInner = new StoryboardManager::TimelineTrack(type);
        return S_OK;
    }

    StoryboardManager::TimelineTrack* GetInner()
    {
        if (!m_pInner)
            m_pInner = new StoryboardManager::TimelineTrack();
        return m_pInner;
    }

    StoryboardManager::TimelineTrackType STDMETHODCALLTYPE GetTrackType()
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (!p) return StoryboardManager::TimelineTrackTypeUnknown;
        return p->GetTrackType();
    }

    LPCWSTR STDMETHODCALLTYPE GetDisplayName()
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (!p) return L"";
        static ATL::CString sName;
        sName = p->GetDisplayName();
        return sName.GetString();
    }

    size_t STDMETHODCALLTYPE GetExtentCount()
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (!p) return 0;
        return p->GetExtentCount();
    }

    HRESULT STDMETHODCALLTYPE AddExtent(DWORD dwExtentId)
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        p->AddExtent(dwExtentId);
        return S_OK;
    }

    void STDMETHODCALLTYPE RemoveAllExtents()
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (p) p->RemoveAllExtents();
    }

    bool STDMETHODCALLTYPE IsVisible()
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (!p) return false;
        return p->IsVisible();
    }

    void STDMETHODCALLTYPE SetVisible(bool fVisible)
    {
        StoryboardManager::TimelineTrack* p = GetInner();
        if (p) p->SetVisible(fVisible);
    }

private:
    StoryboardManager::TimelineTrack* m_pInner;
};

// ============================================================================
// CComSerializationWriter
// ============================================================================
// ATL COM wrapper for StoryboardManager::SerializationWriter.
// Provides an ATL-creatable object for .wlmp project file writing.
//
class ATL_NO_VTABLE CComSerializationWriter :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CComSerializationWriter, &CLSID_SerializationWriterObj>
{
public:
    CComSerializationWriter()
        : m_pInner(NULL)
    {
    }

    ~CComSerializationWriter()
    {
        delete m_pInner;
        m_pInner = NULL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_SERIALIZATION_WRITER_OBJ)
    DECLARE_NOT_AGGREGATABLE(CComSerializationWriter)

    // Manual COM map (avoids ATL CComQIPtr<IUnknown> template conflict in MSVC 14.44)
    BEGIN_COM_MAP(CComSerializationWriter)
        COM_INTERFACE_ENTRY_IID(IID_IUnknown, CComSerializationWriter)
    END_COM_MAP()

    StoryboardManager::SerializationWriter* GetInner()
    {
        if (!m_pInner)
            m_pInner = new StoryboardManager::SerializationWriter();
        return m_pInner;
    }

    HRESULT STDMETHODCALLTYPE WriteToFile(LPCWSTR pszFilePath, IUnknown* pProjectUnknown)
    {
        if (!pszFilePath || !pProjectUnknown)
            return E_INVALIDARG;

        CComPtr<IUnknown> spProject(pProjectUnknown);
        if (!spProject)
            return E_INVALIDARG;

        // Extract the inner MovieProject from the COM wrapper
        CComMovieProject* pMovieProject = reinterpret_cast<CComMovieProject*>(spProject.p);
        if (!pMovieProject)
            return E_INVALIDARG;

        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;

        StoryboardManager::MovieProject* pProject = pMovieProject->GetInner();
        if (!pProject) return E_INVALIDARG;

        return p->WriteToFile(pszFilePath, pProject);
    }

    HRESULT STDMETHODCALLTYPE BeginDocument()
    {
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->BeginDocument();
    }

    HRESULT STDMETHODCALLTYPE EndDocument()
    {
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->EndDocument();
    }

    HRESULT STDMETHODCALLTYPE BeginElement(LPCWSTR pszName)
    {
        if (!pszName) return E_INVALIDARG;
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->BeginElement(pszName);
    }

    HRESULT STDMETHODCALLTYPE EndElement()
    {
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->EndElement();
    }

    HRESULT STDMETHODCALLTYPE WriteAttribute(LPCWSTR pszName, LPCWSTR pszValue)
    {
        if (!pszName) return E_INVALIDARG;
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->WriteAttribute(pszName, pszValue);
    }

    HRESULT STDMETHODCALLTYPE WriteAttributeDWORD(LPCWSTR pszName, DWORD dwValue)
    {
        if (!pszName) return E_INVALIDARG;
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->WriteAttribute(pszName, dwValue);
    }

    HRESULT STDMETHODCALLTYPE WriteAttributeInt(LPCWSTR pszName, int nValue)
    {
        if (!pszName) return E_INVALIDARG;
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->WriteAttribute(pszName, nValue);
    }

    HRESULT STDMETHODCALLTYPE WriteAttributeBool(LPCWSTR pszName, bool fValue)
    {
        if (!pszName) return E_INVALIDARG;
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->WriteAttribute(pszName, fValue);
    }

    HRESULT STDMETHODCALLTYPE Flush()
    {
        StoryboardManager::SerializationWriter* p = GetInner();
        if (!p) return E_OUTOFMEMORY;
        return p->Flush();
    }

private:
    StoryboardManager::SerializationWriter* m_pInner;
};

// ============================================================================
// Object map for ATL module registration
// ============================================================================
BEGIN_OBJECT_MAP(MovieCoreObjectMap)
    OBJECT_ENTRY(CLSID_StoryboardManagerObj, CComStoryboardManager)
    OBJECT_ENTRY(CLSID_MovieProjectObj,       CComMovieProject)
    OBJECT_ENTRY(CLSID_TimelineTrackObj,      CComTimelineTrack)
    OBJECT_ENTRY(CLSID_SerializationWriterObj, CComSerializationWriter)
END_OBJECT_MAP()

#endif // MOVIECORE_COM_FACTORY_H
