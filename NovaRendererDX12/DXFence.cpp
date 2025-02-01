#include "stdafx.h"
#include "DXDevice.h"
#include "DXError.h"
#include "DXHelper.h"
#include "DXFence.h"

namespace Nova {
	DXFence::DXFence() {
		m_Event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	}

	DXFence::~DXFence() {
		CloseHandle(m_Event);
	}

	void DXFence::Create(Microsoft::WRL::ComPtr<ID3D12Device>& device, const char* debugName) {
		m_FenceCounter = 0;
		ThrowIfFailed(device->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_Fence)
		));

		SetName(m_Fence.Get(), debugName);
	}

	void DXFence::Destroy() {
		m_Fence.Reset();
	}

	void DXFence::IssueFence(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue) {
		m_FenceCounter += 1;
		ThrowIfFailed(commandQueue->Signal(m_Fence.Get(), m_FenceCounter));
	}
	
	void DXFence::CPUWaitForFence(UINT64 givenVal) {
		if (m_FenceCounter > givenVal) {
			UINT64 waitValue = m_FenceCounter - givenVal;

			if (m_Fence->GetCompletedValue() <= waitValue) {
				ThrowIfFailed(m_Fence->SetEventOnCompletion(waitValue, m_Event));
				WaitForSingleObject(m_Event, INFINITE);
			}
		}
	}
	
	void DXFence::GPUWaitForFence(const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue) {
		ThrowIfFailed(commandQueue->Wait(m_Fence.Get(), m_FenceCounter));
	}
}