#include "stdafx.h"
#include "DXDevice.h"
#include "DXCommandList.h"
#include "DXHelper.h"
#include "DXError.h"

namespace Nova {
	void DXCommandList::Create(
		DXDevice* device,
		uint32_t numberOfBackBuffers,
		uint32_t commandListsPerBackBuffer,
		const D3D12_COMMAND_QUEUE_DESC& queueDesc
	) {
		m_NumberOfAllocators = numberOfBackBuffers;
		m_CommandListsPerBackBuffer = commandListsPerBackBuffer;

		m_CommandBuffers.resize(m_NumberOfAllocators);

		for (uint32_t idx = 0; idx < m_NumberOfAllocators; idx++) {
			CommandBuffersPerFrame& commandBuffersPerFrame = m_CommandBuffers[idx];

			ThrowIfFailed(device->GetDevice()->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&commandBuffersPerFrame.m_CommandAllocator)));
			SetName(commandBuffersPerFrame.m_CommandAllocator.Get(), format("CommandAllocator %u", idx));

			commandBuffersPerFrame.m_CommandLists.resize(m_CommandListsPerBackBuffer);

			for (uint32_t i = 0; i < m_CommandListsPerBackBuffer; i++) {
				ThrowIfFailed(device->GetDevice()->CreateCommandList(0, queueDesc.Type, commandBuffersPerFrame.m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandBuffersPerFrame.m_CommandLists[i])));

				commandBuffersPerFrame.m_CommandLists[i]->Close();
				SetName(commandBuffersPerFrame.m_CommandLists[i].Get(), format("CommandList %u, Allocator %u", i, idx));
			}

			commandBuffersPerFrame.m_UsedCls = 0;
		}

		ID3D12CommandQueue* queue = (queueDesc.Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
			? device->GetComputeQueue().Get()
			: device->GetGraphicsQueue().Get();

		for (uint32_t idx = 0; idx < m_NumberOfAllocators; idx++) {
			std::vector<ID3D12CommandList*> commandLists;
			commandLists.reserve(m_CommandListsPerBackBuffer);

			for (auto& commandList : m_CommandBuffers[idx].m_CommandLists) {
				commandLists.push_back(commandList.Get());
			}

			queue->ExecuteCommandLists(static_cast<UINT>(commandLists.size()), commandLists.data());
		}

		device->GPUFlush();

		m_FrameIndex = 0;
		m_CurrentFrame = &m_CommandBuffers[m_FrameIndex % m_NumberOfAllocators];
		++m_FrameIndex;
		m_CurrentFrame->m_UsedCls = 0;
	}

	// TODO: not needed because of ComPtrs and vectors, they handle things automatically
	void DXCommandList::Destroy() {}

	void DXCommandList::BeginFrame() {
		m_CurrentFrame = &m_CommandBuffers[m_FrameIndex % m_NumberOfAllocators];
		ThrowIfFailed(m_CurrentFrame->m_CommandAllocator->Reset());

		m_CurrentFrame->m_UsedCls = 0;
		m_FrameIndex++;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> DXCommandList::GetNewCommandList() {
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList = m_CurrentFrame->m_CommandLists[m_CurrentFrame->m_UsedCls];
		m_CurrentFrame->m_UsedCls++;

		assert(m_CurrentFrame->m_UsedCls < m_CommandListsPerBackBuffer);

		ThrowIfFailed(commandList->Reset(m_CurrentFrame->m_CommandAllocator.Get(), nullptr));

		return commandList;
	}
}