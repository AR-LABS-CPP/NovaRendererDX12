#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

namespace Nova {
	class DXDevice {
	public:
		void Create(
			const char* appName,
			const char* engine,
			bool validationEnabled,
			bool gpuValidationEnabled
		);

		void Destroy();

		ID3D12Device* GetDevice() {
			return m_Device.Get();
		}

		IDXGIAdapter* GetAdapter() {
			return m_Adapter.Get();
		}
		
		ID3D12CommandQueue* GetGraphicsQueue() {
			return m_DirectQueue.Get();
		}

		ID3D12CommandQueue* GetComputeQueue() {
			return m_ComputeQueue.Get();
		}

		bool IsFp16Supported() { 
			return m_fp16Supported;
		}
		
		bool IsRT10Supported() { 
			return m_rt10Supported;
		}
		
		bool IsRT11Supported() { 
			return m_rt11Supported;
		}
		
		bool IsVRSTier1Supported() { 
			return m_vrs1Supported;
		}
		
		bool IsVRSTier2Supported() { 
			return m_vrs2Supported;
		}
		
		bool IsBarycentricsSupported() { 
			return m_barycentricsSupported;
		}

		void GetDeviceInfo(std::string* deviceName, std::string* driverVersion);
		void GPUFlush(D3D12_COMMAND_LIST_TYPE cmdType);
		void GPUFlush();
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_Adapter;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_DirectQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_ComputeQueue;

		bool m_fp16Supported = false;
		bool m_rt10Supported = false;
		bool m_rt11Supported = false;
		bool m_vrs1Supported = false;
		bool m_vrs2Supported = false;
		bool m_barycentricsSupported = false;
	};
}