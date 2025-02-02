#include "stdafx.h"
#include "DXError.h"
#include "DXDevice.h"
#include "DXHelper.h"
#include "DXSwapChain.h"

namespace Nova {
	void DXSwapChain::Create(const DXDevice& device, uint32_t backBufferCount, HWND windowHandle) {
		m_WindowHandle = windowHandle;
		m_Device = device.GetDevice();
		m_DirectQueue = device.GetGraphicsQueue();
		m_BackBufferCount = backBufferCount;

		CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));

		// describe the swap chain
		m_SwapChainDesc = {};
		m_SwapChainDesc.BufferCount = m_BackBufferCount;
		m_SwapChainDesc.Width = 0;
		m_SwapChainDesc.Height = 0;
		m_SwapChainDesc.Format = GetFormat();
		m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		m_SwapChainDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_Factory->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&m_TearingSupport,
			sizeof(m_TearingSupport)
		));

		m_SwapChainDesc.Flags = m_TearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		m_SwapChainFence.Create(m_Device, format("SwapChain Fence").c_str());

		IDXGISwapChain1* swapChain;
		ThrowIfFailed(m_Factory->CreateSwapChainForHwnd(
			m_DirectQueue.Get(),
			m_WindowHandle,
			&m_SwapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// Disable alt-enter key combo
		ThrowIfFailed(m_Factory->MakeWindowAssociation(m_WindowHandle, DXGI_MWA_NO_ALT_ENTER));
		ThrowIfFailed(swapChain->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_SwapChain));

		// Explicity reset so that ComPtr can clean it
		swapChain->Release();
		
		D3D12_DESCRIPTOR_HEAP_DESC descHeapRTV;
		descHeapRTV.NumDescriptors = m_SwapChainDesc.BufferCount;
		descHeapRTV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapRTV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapRTV.NodeMask = 0;

		ThrowIfFailed(m_Device->CreateDescriptorHeap(&descHeapRTV, IID_PPV_ARGS(&m_RTVHeaps)));
		
		CreateRenderTargetView();
	}

	void DXSwapChain::Destroy() {
		m_SwapChainFence.Destroy();
		m_RTVHeaps.Reset();
		m_SwapChain.Reset();
		m_Factory.Reset();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* DXSwapChain::GetCurrentBackBufferRTV() {
		uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
		return &m_CPUView[backBufferIndex];
	}

	void DXSwapChain::WaitForSwapChain() {
		uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
		m_SwapChainFence.CPUWaitForFence(m_BackBufferCount - 1);
	}

	void DXSwapChain::CreateWindowSizeDependentResource(uint32_t width, uint32_t height, bool Vsync, bool disableLocalDimming) {
		ThrowIfFailed(m_SwapChain->ResizeBuffers(
			m_SwapChainDesc.BufferCount,
			width,
			height,
			GetFormat(),
			m_SwapChainDesc.Flags
		));

		CreateRenderTargetView();
	}

	void DXSwapChain::DestroyWindowSizeDependentResource() {
		m_CPUView.clear();
		m_RTVHeaps.Reset();
		m_SwapChain.Reset();
	}

	void DXSwapChain::SetFullScreen(bool fullscreen) {
		ThrowIfFailed(m_SwapChain->SetFullscreenState(fullscreen, nullptr));
		m_IsFullScreenExclusive = fullscreen;
	}
	
	const bool DXSwapChain::GetFullScreen() {
		BOOL fullscreen = FALSE;
		HRESULT hr = m_SwapChain->GetFullscreenState(&fullscreen, nullptr);
		
		if (FAILED(hr)) {
			return false;
		}
		
		m_IsFullScreenExclusive = fullscreen;
		return m_IsFullScreenExclusive;
	}

	void DXSwapChain::PresentSwapChain() {
		if (m_Vsync) {
			ThrowIfFailed(m_SwapChain->Present(1, 0));
			return;
		}

		UINT presentFlags = m_TearingSupport && !m_IsFullScreenExclusive
			? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(m_SwapChain->Present(0, presentFlags));

		// Issue a fence so we can tell when this frame ended
		m_SwapChainFence.IssueFence(m_DirectQueue);
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> DXSwapChain::GetCurrentBackBufferResource() {
		uint32_t backBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		ID3D12Resource* backBuffer;
		ThrowIfFailed(m_SwapChain->GetBuffer(backBufferIndex, IID_PPV_ARGS(&backBuffer)));
		backBuffer->Release();

		return backBuffer;
	}
	
	// TODO: Extend this in case you are supporting multiple formats
	DXGI_FORMAT DXSwapChain::GetFormat() {
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	void DXSwapChain::CreateRenderTargetView() {
		uint32_t colorDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
		m_CPUView.resize(m_SwapChainDesc.BufferCount);

		for (uint32_t idx = 0; idx < m_SwapChainDesc.BufferCount; idx++) {
			m_CPUView[idx] = m_RTVHeaps->GetCPUDescriptorHandleForHeapStart();
			m_CPUView[idx].ptr += colorDescriptorSize * idx;

			ID3D12Resource* backBuffer;
			ThrowIfFailed(m_SwapChain->GetBuffer(idx, IID_PPV_ARGS(&backBuffer)));
			SetName(backBuffer, "SwapChain");
			
			D3D12_RENDER_TARGET_VIEW_DESC colorDesc = {};
			colorDesc.Format = GetFormat();
			colorDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			colorDesc.Texture2D.MipSlice = 0;
			colorDesc.Texture2D.PlaneSlice = 0;

			m_Device->CreateRenderTargetView(backBuffer, &colorDesc, m_CPUView[idx]);
			SetName(backBuffer, format("BackBuffer %idx", idx));
			backBuffer->Release();
		}
	}
}