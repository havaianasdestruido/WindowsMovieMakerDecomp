#pragma once
// TextRenderPipeline.h - DirectWrite text rendering task pipeline

#include "../HMREngine.h"
#include "../DXResources/DXResources.h"
#include <dwrite.h>
#include <d2d1.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>

namespace HMREngine
{
    // --- TaskFinishNotificationCallback ---
    class TaskFinishNotificationCallback
    {
    public:
        TaskFinishNotificationCallback();
        virtual ~TaskFinishNotificationCallback();

        virtual void OnTaskFinished(class TextRenderTask* task) = 0;
    };

    // --- TextRenderTask: Base class for all text render tasks ---
    class TextRenderTask
    {
    public:
        enum class TaskState
        {
            Pending,
            Running,
            Completed,
            Failed,
            Cancelled,
        };

        TextRenderTask();
        virtual ~TextRenderTask();

        virtual HRESULT Execute() = 0;
        virtual void Cancel();

        TaskState GetState() const { return m_state; }
        bool IsComplete() const { return m_state == TaskState::Completed || m_state == TaskState::Failed || m_state == TaskState::Cancelled; }

        void SetCallback(TaskFinishNotificationCallback* cb) { m_callback = cb; }
        TaskFinishNotificationCallback* GetCallback() const { return m_callback; }

        HRESULT GetResult() const { return m_result; }

        void SetWidth(UINT w) { m_width = w; }
        void SetHeight(UINT h) { m_height = h; }
        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }

        ID3D11Texture2D* GetResultTexture() const { return m_resultTexture; }
        ID3D11ShaderResourceView* GetResultSRV() const { return m_resultSRV; }

    protected:
        TaskState m_state = TaskState::Pending;
        HRESULT m_result = S_OK;
        TaskFinishNotificationCallback* m_callback = nullptr;
        CComPtr<ID3D11Texture2D> m_resultTexture;
        CComPtr<ID3D11ShaderResourceView> m_resultSRV;
        UINT m_width = 0;
        UINT m_height = 0;
    };

    // --- PartialTextRenderTask: Partial/incremental text rendering ---
    class PartialTextRenderTask : public TextRenderTask
    {
    public:
        PartialTextRenderTask();
        virtual ~PartialTextRenderTask();

        HRESULT Execute() override;

        void SetText(const std::wstring& text) { m_text = text; }
        void SetPosition(float x, float y) { m_x = x; m_y = y; }
        void SetFontFamily(const std::wstring& family) { m_fontFamily = family; }
        void SetFontSize(float size) { m_fontSize = size; }
        void SetColor(const Rgba& color) { m_color = color; }
        void SetBold(bool bold) { m_bold = bold; }
        void SetItalic(bool italic) { m_italic = italic; }

    protected:
        std::wstring m_text;
        float m_x = 0.0f;
        float m_y = 0.0f;
        std::wstring m_fontFamily = L"Segoe UI";
        float m_fontSize = 24.0f;
        Rgba m_color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
        bool m_bold = false;
        bool m_italic = false;
    };

    // --- PartialTextPreRenderTask: Pre-render step for partial text ---
    class PartialTextPreRenderTask : public TextRenderTask
    {
    public:
        PartialTextPreRenderTask();
        virtual ~PartialTextPreRenderTask();

        HRESULT Execute() override;

        void SetText(const std::wstring& text) { m_text = text; }
        void SetFontFamily(const std::wstring& family) { m_fontFamily = family; }
        void SetFontSize(float size) { m_fontSize = size; }

        float GetMeasuredWidth() const { return m_measuredWidth; }
        float GetMeasuredHeight() const { return m_measuredHeight; }

    protected:
        std::wstring m_text;
        std::wstring m_fontFamily = L"Segoe UI";
        float m_fontSize = 24.0f;
        float m_measuredWidth = 0.0f;
        float m_measuredHeight = 0.0f;
    };

    // --- TextBitmapRenderTask: Render text to bitmap ---
    class TextBitmapRenderTask : public TextRenderTask
    {
    public:
        TextBitmapRenderTask();
        virtual ~TextBitmapRenderTask();

        HRESULT Execute() override;

        void SetText(const std::wstring& text) { m_text = text; }
        void SetFontFamily(const std::wstring& family) { m_fontFamily = family; }
        void SetFontSize(float size) { m_fontSize = size; }
        void SetColor(const Rgba& color) { m_color = color; }
        void SetBold(bool bold) { m_bold = bold; }
        void SetItalic(bool italic) { m_italic = italic; }
        void SetBackgroundColor(const Rgba& bg) { m_backgroundColor = bg; }

        const std::vector<BYTE>& GetBitmapData() const { return m_bitmapData; }

    protected:
        std::wstring m_text;
        std::wstring m_fontFamily = L"Segoe UI";
        float m_fontSize = 24.0f;
        Rgba m_color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
        Rgba m_backgroundColor = Rgba(0.0f, 0.0f, 0.0f, 0.0f);
        bool m_bold = false;
        bool m_italic = false;
        std::vector<BYTE> m_bitmapData;
    };

    // --- FullTextRenderTask: Full text render to texture ---
    class FullTextRenderTask : public TextRenderTask
    {
    public:
        FullTextRenderTask();
        virtual ~FullTextRenderTask();

        HRESULT Execute() override;

        void SetText(const std::wstring& text) { m_text = text; }
        void SetFontFamily(const std::wstring& family) { m_fontFamily = family; }
        void SetFontSize(float size) { m_fontSize = size; }
        void SetColor(const Rgba& color) { m_color = color; }
        void SetBold(bool bold) { m_bold = bold; }
        void SetItalic(bool italic) { m_italic = italic; }
        void SetAlignment(UINT align) { m_alignment = align; }

    protected:
        std::wstring m_text;
        std::wstring m_fontFamily = L"Segoe UI";
        float m_fontSize = 24.0f;
        Rgba m_color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
        bool m_bold = false;
        bool m_italic = false;
        UINT m_alignment = 0;
    };

} // namespace HMREngine
