#pragma once
// DeviceClientDX.h - Base class for device-dependent resources

#include "DXResources.h"

namespace HMREngine
{
    namespace DX
    {
        class EngineDX;

        // --- DeviceClientDXImpl: Base for device-dependent resources ---
        class DeviceClientDXImpl
        {
        public:
            DeviceClientDXImpl();
            virtual ~DeviceClientDXImpl();

            virtual HRESULT InitializeDevice(ID3D11Device* dev, ID3D11DeviceContext* ctx);
            virtual void ReleaseDevice();

            virtual HRESULT OnDeviceLost();
            virtual HRESULT OnDeviceRestored(ID3D11Device* dev, ID3D11DeviceContext* ctx);

            ID3D11Device* GetDevice() const { return m_device; }
            ID3D11DeviceContext* GetContext() const { return m_context; }
            bool HasDevice() const { return m_device != nullptr; }

        protected:
            ID3D11Device* m_device = nullptr;
            ID3D11DeviceContext* m_context = nullptr;
            EngineDX* m_engine = nullptr;
        };

    } // namespace DX
} // namespace HMREngine
