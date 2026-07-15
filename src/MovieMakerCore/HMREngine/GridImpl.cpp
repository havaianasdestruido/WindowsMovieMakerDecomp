#include "pch.h"

// GridImpl.cpp - Grid/GridNode/GridShader implementations

#include "GridImpl.h"

namespace HMREngine
{
    // GridImpl
    GridImpl::GridImpl()
    {
    }

    GridImpl::~GridImpl()
    {
        Shutdown();
    }

    HRESULT GridImpl::Initialize(ID3D11Device* dev)
    {
        if (!dev) return E_POINTER;
        BuildBuffer(dev);
        return S_OK;
    }

    void GridImpl::Shutdown()
    {
        m_vertexBuffer.Release();
    }

    void GridImpl::SetGridParams(float width, float height, int divisions)
    {
        m_width = width;
        m_height = height;
        m_divisions = divisions;
    }

    void GridImpl::SetLineColor(const Rgb& color)
    {
        m_lineColor = color;
    }

    void GridImpl::SetLineWidth(float width)
    {
        m_lineWidth = width;
    }

    void GridImpl::BuildBuffer(ID3D11Device* dev)
    {
        struct Vertex { Vec3 pos; };

        std::vector<Vertex> verts;

        float halfW = m_width * 0.5f;
        float halfH = m_height * 0.5f;
        float stepX = m_width / m_divisions;
        float stepY = m_height / m_divisions;

        // Vertical lines
        for (int i = 0; i <= m_divisions; i++)
        {
            float x = -halfW + i * stepX;
            verts.push_back({ Vec3(x, 0, -halfH) });
            verts.push_back({ Vec3(x, 0, halfH) });
        }

        // Horizontal lines
        for (int i = 0; i <= m_divisions; i++)
        {
            float z = -halfH + i * stepY;
            verts.push_back({ Vec3(-halfW, 0, z) });
            verts.push_back({ Vec3(halfW, 0, z) });
        }

        m_vertexCount = static_cast<UINT>(verts.size());

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = m_vertexCount * sizeof(Vertex);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = verts.data();

        m_vertexBuffer.Release();
        dev->CreateBuffer(&bd, &initData, &m_vertexBuffer);
    }

    void GridImpl::Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj)
    {
        if (!m_vertexBuffer || !ctx) return;

        UINT stride = sizeof(Vec3);
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &stride, &offset);
        ctx->Draw(m_vertexCount, 0);
    }

    // GridNodeImpl
    HRESULT GridNodeImpl::Initialize(GridNode* node, ID3D11Device* dev)
    {
        m_node = node;
        m_grid = new GridImpl();
        m_grid->SetGridParams(node->m_width, node->m_height, node->m_divisions);
        m_grid->SetLineColor(node->m_lineColor);
        m_grid->SetLineWidth(node->m_lineWidth);
        return m_grid->Initialize(dev);
    }

    void GridNodeImpl::Render(ID3D11DeviceContext* ctx, const Matrix4f& worldViewProj)
    {
        if (m_grid) m_grid->Render(ctx, worldViewProj);
    }

} // namespace HMREngine
