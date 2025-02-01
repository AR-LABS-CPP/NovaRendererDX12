#pragma once

namespace Nova {
	class DXFence {
	public:
		DXFence();
		~DXFence();

		void Create(Microsoft::WRL::ComPtr<ID3D12Device>& device, const char* debugName);
		void Destroy();

		void IssueFence(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue);
		void CPUWaitForFence(UINT64 givenVal);
		void GPUWaitForFence(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue);
	private:
		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence = nullptr;
		HANDLE m_Event = nullptr;
		UINT64 m_FenceCounter = 0;
	};
}