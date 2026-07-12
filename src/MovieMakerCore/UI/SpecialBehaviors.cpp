/*
 * SpecialBehaviors.cpp
 *
 * Implementation of special-purpose DirectUI behaviors.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SpecialBehaviors.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// HomerHeavyLayerBehavior
// ============================================================================
HomerHeavyLayerBehavior::HomerHeavyLayerBehavior()
    : m_pElement(NULL), m_pAppMain(NULL), m_pHomerScene(NULL)
    , m_pPreviewSurface(NULL), m_bLayerActive(false)
{
    SetRectEmpty(&m_rcRender);
}

HomerHeavyLayerBehavior::~HomerHeavyLayerBehavior() { m_pElement = NULL; }

HRESULT HomerHeavyLayerBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT HomerHeavyLayerBehavior::OnElementDetached(IDuiElement*)
{
    m_pElement = NULL;
    m_pHomerScene = NULL;
    m_pPreviewSurface = NULL;
    return S_OK;
}

HRESULT HomerHeavyLayerBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    if (!m_bLayerActive)
        return S_OK;

    // Render the Homer scene into the layer
    // In the full implementation, this would call into the HMREngine
    // to render the current X3D scene into the specified HDC/bounds.
    // For now, fill with a dark background representing the preview area.
    HBRUSH hBrush = CreateSolidBrush(RGB(16, 16, 16));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    return S_OK;
}

HRESULT HomerHeavyLayerBehavior::BeginLayer(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    m_bLayerActive = true;
    return S_OK;
}

HRESULT HomerHeavyLayerBehavior::EndLayer(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    m_bLayerActive = false;
    return S_OK;
}

HRESULT HomerHeavyLayerBehavior::InvalidateLayer()
{
    if (m_pElement)
        m_pElement->InvalidateRect(&m_rcRender);
    return S_OK;
}

void HomerHeavyLayerBehavior::SetHomerScene(void* pScene) { m_pHomerScene = pScene; }
void HomerHeavyLayerBehavior::SetPreviewSurface(void* pSurface) { m_pPreviewSurface = pSurface; }
void HomerHeavyLayerBehavior::SetRenderRect(const RECT& rcRender) { m_rcRender = rcRender; }

// ============================================================================
// StandardLayerBehavior
// ============================================================================
StandardLayerBehavior::StandardLayerBehavior()
    : m_pElement(NULL), m_hdcMem(NULL), m_hBmpMem(NULL)
    , m_hBmpOld(NULL), m_nWidth(0), m_nHeight(0)
{
}

StandardLayerBehavior::~StandardLayerBehavior()
{
    if (m_hdcMem)
    {
        if (m_hBmpOld) SelectObject(m_hdcMem, m_hBmpOld);
        DeleteDC(m_hdcMem);
    }
    if (m_hBmpMem) DeleteObject(m_hBmpMem);
    m_pElement = NULL;
}

HRESULT StandardLayerBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT StandardLayerBehavior::OnElementDetached(IDuiElement*)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT StandardLayerBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    return S_OK;
}

HRESULT StandardLayerBehavior::BeginLayer(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    int nWidth = prcBounds->right - prcBounds->left;
    int nHeight = prcBounds->bottom - prcBounds->top;

    if (nWidth != m_nWidth || nHeight != m_nHeight || !m_hdcMem)
    {
        if (m_hdcMem) { if (m_hBmpOld) SelectObject(m_hdcMem, m_hBmpOld); DeleteDC(m_hdcMem); }
        if (m_hBmpMem) DeleteObject(m_hBmpMem);

        m_hdcMem = CreateCompatibleDC(hdc);
        m_hBmpMem = CreateCompatibleBitmap(hdc, nWidth, nHeight);
        m_hBmpOld = static_cast<HBITMAP>(SelectObject(m_hdcMem, m_hBmpMem));
        m_nWidth = nWidth;
        m_nHeight = nHeight;
    }

    return S_OK;
}

HRESULT StandardLayerBehavior::EndLayer(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds || !m_hdcMem) return E_POINTER;

    BitBlt(hdc, prcBounds->left, prcBounds->top,
        prcBounds->right - prcBounds->left,
        prcBounds->bottom - prcBounds->top,
        m_hdcMem, 0, 0, SRCCOPY);

    return S_OK;
}

HRESULT StandardLayerBehavior::InvalidateLayer()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
    return S_OK;
}

// ============================================================================
// MultipleEffectBehavior
// ============================================================================
MultipleEffectBehavior::MultipleEffectBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
MultipleEffectBehavior::~MultipleEffectBehavior() { m_pElement = NULL; }

HRESULT MultipleEffectBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT MultipleEffectBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT MultipleEffectBehavior::AddEffect(DWORD dwEffectId)
{
    EffectEntry entry;
    entry.dwEffectId = dwEffectId;
    entry.nIndex = static_cast<int>(m_effectStack.size());
    m_effectStack.push_back(entry);
    return S_OK;
}

HRESULT MultipleEffectBehavior::RemoveEffect(DWORD dwEffectId)
{
    for (auto it = m_effectStack.begin(); it != m_effectStack.end(); ++it)
    {
        if (it->dwEffectId == dwEffectId)
        {
            m_effectStack.erase(it);
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT MultipleEffectBehavior::MoveEffect(DWORD dwEffectId, int nNewIndex)
{
    for (size_t i = 0; i < m_effectStack.size(); ++i)
    {
        if (m_effectStack[i].dwEffectId == dwEffectId)
        {
            EffectEntry entry = m_effectStack[i];
            m_effectStack.erase(m_effectStack.begin() + i);
            if (nNewIndex < 0) nNewIndex = 0;
            if (nNewIndex >= static_cast<int>(m_effectStack.size()))
                nNewIndex = static_cast<int>(m_effectStack.size());
            m_effectStack.insert(m_effectStack.begin() + nNewIndex, entry);
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

DWORD MultipleEffectBehavior::GetEffectCount() const throw() { return static_cast<DWORD>(m_effectStack.size()); }

HRESULT MultipleEffectBehavior::GetEffectId(int nIndex, DWORD* pdwEffectId)
{
    if (!pdwEffectId) return E_POINTER;
    if (nIndex < 0 || nIndex >= static_cast<int>(m_effectStack.size()))
        return E_BOUNDS;
    *pdwEffectId = m_effectStack[nIndex].dwEffectId;
    return S_OK;
}

HRESULT MultipleEffectBehavior::SetEffectParameter(DWORD dwEffectId, LPCWSTR pszParamName, float fValue)
{
    if (!pszParamName) return E_INVALIDARG;
    for (size_t i = 0; i < m_effectStack.size(); ++i)
    {
        if (m_effectStack[i].dwEffectId == dwEffectId)
        {
            m_effectStack[i].parameters[ATL::CString(pszParamName)] = fValue;
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

HRESULT MultipleEffectBehavior::GetEffectParameter(DWORD dwEffectId, LPCWSTR pszParamName, float* pfValue)
{
    if (!pszParamName || !pfValue) return E_INVALIDARG;
    for (size_t i = 0; i < m_effectStack.size(); ++i)
    {
        if (m_effectStack[i].dwEffectId == dwEffectId)
        {
            auto it = m_effectStack[i].parameters.find(ATL::CString(pszParamName));
            if (it != m_effectStack[i].parameters.end())
            {
                *pfValue = it->second;
                return S_OK;
            }
        }
    }
    return E_INVALIDARG;
}

// ============================================================================
// WebcamElementBehavior
// ============================================================================
WebcamElementBehavior::WebcamElementBehavior()
    : m_pElement(NULL), m_hWndHost(NULL), m_bPreviewing(false), m_hCurrentFrame(NULL)
{
}

WebcamElementBehavior::~WebcamElementBehavior()
{
    StopPreview();
    if (m_hCurrentFrame) { DeleteObject(m_hCurrentFrame); m_hCurrentFrame = NULL; }
    m_pElement = NULL;
}

HRESULT WebcamElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT WebcamElementBehavior::OnElementDetached(IDuiElement*) { StopPreview(); m_pElement = NULL; return S_OK; }

HRESULT WebcamElementBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    if (m_hCurrentFrame)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOld = static_cast<HBITMAP>(SelectObject(hdcMem, m_hCurrentFrame));
        BITMAP bm;
        GetObject(m_hCurrentFrame, sizeof(bm), &bm);
        StretchBlt(hdc, prcBounds->left, prcBounds->top,
            prcBounds->right - prcBounds->left, prcBounds->bottom - prcBounds->top,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);
    }
    else
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(32, 32, 32));
        FillRect(hdc, prcBounds, hBrush);
        DeleteObject(hBrush);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(150, 150, 150));
        DrawText(hdc, L"Webcam Preview", -1, const_cast<RECT*>(prcBounds),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    return S_OK;
}

HRESULT WebcamElementBehavior::StartPreview(HWND hWndHost)
{
    m_hWndHost = hWndHost;
    m_bPreviewing = true;
    if (m_pElement) m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HRESULT WebcamElementBehavior::StopPreview()
{
    m_bPreviewing = false;
    if (m_hCurrentFrame) { DeleteObject(m_hCurrentFrame); m_hCurrentFrame = NULL; }
    if (m_pElement) m_pElement->InvalidateRect(NULL);
    return S_OK;
}

bool WebcamElementBehavior::IsPreviewing() const throw() { return m_bPreviewing; }

HRESULT WebcamElementBehavior::CaptureFrame(LPCWSTR pszOutputPath)
{
    UNREFERENCED_PARAMETER(pszOutputPath);
    return S_OK;
}

// ============================================================================
// AmpPreventSparseModeBehavior
// ============================================================================
AmpPreventSparseModeBehavior::AmpPreventSparseModeBehavior() : m_pElement(NULL) {}
AmpPreventSparseModeBehavior::~AmpPreventSparseModeBehavior() { m_pElement = NULL; }

HRESULT AmpPreventSparseModeBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT AmpPreventSparseModeBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// AmpSlideAnimationBehavior
// ============================================================================
AmpSlideAnimationBehavior::AmpSlideAnimationBehavior()
    : m_pElement(NULL), m_direction(SlideLeft)
    , m_dwDuration(300), m_dwStartTime(0)
    , m_bAnimating(false), m_nTimerId(0)
{
}

AmpSlideAnimationBehavior::~AmpSlideAnimationBehavior()
{
    StopSlide();
    m_pElement = NULL;
}

HRESULT AmpSlideAnimationBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT AmpSlideAnimationBehavior::OnElementDetached(IDuiElement*) { StopSlide(); m_pElement = NULL; return S_OK; }

HRESULT AmpSlideAnimationBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_TIMER && wParam == m_nTimerId && m_bAnimating)
    {
        DWORD dwElapsed = GetTickCount() - m_dwStartTime;
        if (dwElapsed >= m_dwDuration)
        {
            m_bAnimating = false;
            KillTimer(NULL, m_nTimerId);
            m_nTimerId = 0;
        }
        if (m_pElement)
            m_pElement->InvalidateRect(NULL);
        if (pbHandled) *pbHandled = TRUE;
    }
    return S_OK;
}

HRESULT AmpSlideAnimationBehavior::StartSlide(SlideDirection direction, DWORD dwDurationMs)
{
    m_direction = direction;
    m_dwDuration = max(1, dwDurationMs);
    m_dwStartTime = GetTickCount();
    m_bAnimating = true;

    if (m_nTimerId == 0)
        m_nTimerId = reinterpret_cast<UINT>(this);

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

HRESULT AmpSlideAnimationBehavior::StopSlide()
{
    m_bAnimating = false;
    if (m_nTimerId)
    {
        KillTimer(NULL, m_nTimerId);
        m_nTimerId = 0;
    }
    return S_OK;
}

bool AmpSlideAnimationBehavior::IsAnimating() const throw() { return m_bAnimating; }

float AmpSlideAnimationBehavior::EaseInOutCubic(float t) const
{
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// ============================================================================
// AmpFadeAnimationBehavior
// ============================================================================
AmpFadeAnimationBehavior::AmpFadeAnimationBehavior()
    : m_pElement(NULL), m_fCurrentAlpha(1.0f)
    , m_fStartAlpha(1.0f), m_fEndAlpha(1.0f)
    , m_dwDuration(300), m_dwStartTime(0)
    , m_bAnimating(false), m_bFadeIn(true), m_nTimerId(0)
{
}

AmpFadeAnimationBehavior::~AmpFadeAnimationBehavior()
{
    if (m_bAnimating && m_nTimerId)
        KillTimer(NULL, m_nTimerId);
    m_pElement = NULL;
}

HRESULT AmpFadeAnimationBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT AmpFadeAnimationBehavior::OnElementDetached(IDuiElement*)
{
    if (m_bAnimating && m_nTimerId) KillTimer(NULL, m_nTimerId);
    m_pElement = NULL;
    return S_OK;
}

HRESULT AmpFadeAnimationBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_TIMER && wParam == m_nTimerId && m_bAnimating)
    {
        DWORD dwElapsed = GetTickCount() - m_dwStartTime;
        float t = min(1.0f, static_cast<float>(dwElapsed) / static_cast<float>(m_dwDuration));
        m_fCurrentAlpha = m_fStartAlpha + (m_fEndAlpha - m_fStartAlpha) * t;

        if (t >= 1.0f)
        {
            m_bAnimating = false;
            KillTimer(NULL, m_nTimerId);
            m_nTimerId = 0;
            m_fCurrentAlpha = m_fEndAlpha;
        }

        if (m_pElement)
            m_pElement->InvalidateRect(NULL);
        if (pbHandled) *pbHandled = TRUE;
    }
    return S_OK;
}

HRESULT AmpFadeAnimationBehavior::FadeIn(DWORD dwDurationMs)
{
    m_bFadeIn = true;
    m_fStartAlpha = 0.0f;
    m_fEndAlpha = 1.0f;
    m_fCurrentAlpha = 0.0f;
    m_dwDuration = max(1, dwDurationMs);
    m_dwStartTime = GetTickCount();
    m_bAnimating = true;
    if (m_nTimerId == 0) m_nTimerId = reinterpret_cast<UINT>(this) + 1;
    if (m_pElement) m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HRESULT AmpFadeAnimationBehavior::FadeOut(DWORD dwDurationMs)
{
    m_bFadeIn = false;
    m_fStartAlpha = 1.0f;
    m_fEndAlpha = 0.0f;
    m_fCurrentAlpha = 1.0f;
    m_dwDuration = max(1, dwDurationMs);
    m_dwStartTime = GetTickCount();
    m_bAnimating = true;
    if (m_nTimerId == 0) m_nTimerId = reinterpret_cast<UINT>(this) + 1;
    if (m_pElement) m_pElement->InvalidateRect(NULL);
    return S_OK;
}

void AmpFadeAnimationBehavior::SetAlpha(float fAlpha) { m_fCurrentAlpha = max(0.0f, min(1.0f, fAlpha)); }
float AmpFadeAnimationBehavior::GetAlpha() const throw() { return m_fCurrentAlpha; }
bool AmpFadeAnimationBehavior::IsAnimating() const throw() { return m_bAnimating; }

// ============================================================================
// UserEncodeProfileBehavior
// ============================================================================
UserEncodeProfileBehavior::UserEncodeProfileBehavior()
    : m_pElement(NULL), m_pAppMain(NULL), m_dwSelectedProfile(0)
{
}

UserEncodeProfileBehavior::~UserEncodeProfileBehavior() { m_pElement = NULL; }

HRESULT UserEncodeProfileBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    LoadProfiles();
    return S_OK;
}

HRESULT UserEncodeProfileBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT UserEncodeProfileBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return S_OK;
}

HRESULT UserEncodeProfileBehavior::LoadProfiles()
{
    m_profiles.clear();

    EncodeProfileEntry profile;
    profile.dwId = 1;
    profile.strName = L"Windows Live Movie Maker (default)";
    profile.bIsCustom = false;
    m_profiles.push_back(profile);

    return S_OK;
}

HRESULT UserEncodeProfileBehavior::SelectProfile(DWORD dwProfileId)
{
    for (size_t i = 0; i < m_profiles.size(); ++i)
    {
        if (m_profiles[i].dwId == dwProfileId)
        {
            m_dwSelectedProfile = dwProfileId;
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

DWORD UserEncodeProfileBehavior::GetSelectedProfile() const throw() { return m_dwSelectedProfile; }

HRESULT UserEncodeProfileBehavior::CreateCustomProfile(LPCWSTR pszName, DWORD* pdwNewProfileId)
{
    if (!pszName || !pdwNewProfileId)
        return E_INVALIDARG;

    DWORD dwNextId = 100;
    for (size_t i = 0; i < m_profiles.size(); ++i)
    {
        if (m_profiles[i].dwId >= dwNextId)
            dwNextId = m_profiles[i].dwId + 1;
    }

    EncodeProfileEntry entry;
    entry.dwId = dwNextId;
    entry.strName = pszName;
    entry.bIsCustom = true;
    m_profiles.push_back(entry);

    *pdwNewProfileId = dwNextId;
    return S_OK;
}

HRESULT UserEncodeProfileBehavior::DeleteCustomProfile(DWORD dwProfileId)
{
    for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it)
    {
        if (it->dwId == dwProfileId && it->bIsCustom)
        {
            m_profiles.erase(it);
            if (m_dwSelectedProfile == dwProfileId)
                m_dwSelectedProfile = 0;
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

DWORD UserEncodeProfileBehavior::GetProfileCount() const throw() { return static_cast<DWORD>(m_profiles.size()); }

HRESULT UserEncodeProfileBehavior::GetProfileName(DWORD dwProfileId, LPWSTR pszName, UINT cchName)
{
    if (!pszName || cchName == 0) return E_INVALIDARG;

    for (size_t i = 0; i < m_profiles.size(); ++i)
    {
        if (m_profiles[i].dwId == dwProfileId)
        {
            wcscpy_s(pszName, cchName, m_profiles[i].strName);
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

} // namespace Sundance
