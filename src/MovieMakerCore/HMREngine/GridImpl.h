#pragma once
// GridImpl.h - Grid/GridNode/GridShader implementations

#include "HMREngine.h"
#include "X3DNodes.h"
#include "Shaders.h"
#include <d3d11.h>
#include <atlbase.h>

namespace HMREngine
{
    class GridImpl : public ImplObject<GridImpl>
    {
    public:
        GridImpl();
        virtual ~GridImpl();

        HRESULT Initialize(ID3D11Device* dev);
        void Shutdown();

        void SetGridParams(float width, float height, int divisions);
        void SetLineColor(const Rgb& color);
        void SetLineWidth(float width);

        void Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj);

        ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer; }
        UINT GetVertexCount() const { return m_vertexCount; }

    private:
        CComPtr<ID3D11Buffer> m_vertexBuffer;
        UINT m_vertexCount = 0;

        float m_width = 1.0f;
        float m_height = 1.0f;
        int m_divisions = 4;
        Rgb m_lineColor = Rgb(0.5f, 0.5f, 0.5f);
        float m_lineWidth = 1.0f;

        void BuildBuffer(ID3D11Device* dev);
    };

    class GridNodeImpl : public ImplObject<GridNodeImpl>
    {
    public:
        SAIPtr<GridImpl> m_grid;
        GridNode* m_node = nullptr;

        HRESULT Initialize(GridNode* node, ID3D11Device* dev);
        void Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj);
    };

} // namespace HMREngine
