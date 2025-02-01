#pragma once

#include <dxgi1_6.h>

namespace Nova {
	class DXSwapChain {
	public:
		void Create(const DXDevice& device, uint32_t backBufferCount, HWND windowHandle);
		void Destroy();

		void CreateWindowSizeDependentResource(uint32_t width, uint32_t height, bool Vsync, bool disableLocalDimming = false);
		void DestroyWindowSizeDependentResource();

		void SetFullScreen(bool fullscreen);
		const bool GetFullScreen();

		void PresentSwapChain();
		void WaitForSwapChain();

		Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBufferResource();
		D3D12_CPU_DESCRIPTOR_HANDLE* GetCurrentBackBufferRTV();
		DXGI_FORMAT GetFormat();

		void SetVSync(bool vsyncVal) { m_Vsync = vsyncVal; }
	private:
		void CreateRenderTargetView();

		HWND m_WindowHandle = NULL;
		uint32_t m_BackBufferCount = 0;

		Microsoft::WRL::ComPtr<ID3D12Device> m_Device = nullptr;
		Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory = nullptr;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_DirectQueue = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeaps = nullptr;

		DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc = {};
		DXFence m_SwapChainFence;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_CPUView;

		bool m_Vsync = false;
		BOOL m_TearingSupport;
		BOOL m_IsFullScreenExclusive;
	};
}