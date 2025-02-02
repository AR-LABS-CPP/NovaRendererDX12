#pragma once

#include <dxgi1_6.h>
#include <dxgidebug.h>

namespace Nova {
	class DXDevice {
	public:
		void Create(
			const char* appName,
			const char* engine,
			bool validationEnabled,
			bool gpuValidationEnabled,
			HWND hWnd
		);

		void Destroy();

		const Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice() const {
			return m_Device;
		}

		const Microsoft::WRL::ComPtr<IDXGIAdapter>& GetAdapter() const {
			return m_Adapter;
		}

		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetGraphicsQueue() const {
			return m_DirectQueue;
		}

		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetComputeQueue() const {
			return m_ComputeQueue;
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