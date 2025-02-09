#include "stdafx.h"
#include "DXDevice.h"
#include "AsyncPool.h"
#include "DXError.h"
#include "DXHelper.h"
#include "DXUploadHeap.h"

namespace Nova {
	void UploadHeap::Create(DXDevice* device, SIZE_T uSize) {
		m_Device = device;
		m_CommandQueue = device->GetGraphicsQueue();

		device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));

		ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadHeap)
		));

		ThrowIfFailed(m_UploadHeap->Map(0, NULL, (void**)&m_DataBegin));

		m_DataCur = m_DataBegin;
		m_DataEnd = m_DataBegin + m_UploadHeap->GetDesc().Width;
	}

	void UploadHeap::Destroy() {
		m_UploadHeap.Reset();
		m_CommandList.Reset();
		m_CommandAllocator.Reset();
	}

	UINT8* UploadHeap::Suballocate(SIZE_T uSize, UINT64 uAlign) {
		flushing.Wait();
		UINT8* ret = NULL;

		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			assert(uSize < (size_t)(m_DataEnd - m_DataBegin));

			m_DataCur = reinterpret_cast<UINT8*>(AlignUp(reinterpret_cast<SIZE_T>(m_DataCur), SIZE_T(uAlign)));
			
			if (m_DataCur >= m_DataEnd || m_DataCur + uSize >= m_DataEnd) {
				return NULL;
			}

			ret = m_DataCur;
			m_DataCur += uSize;
		}

		return ret;
	}

	UINT8* UploadHeap::BeginSuballocate(SIZE_T uSize, UINT64 uAlign) {
		UINT8* res = NULL;

		for (;;) {
			res = Suballocate(uSize, uAlign);

			if (res != NULL) {
				break;
			}

			FlushAndFinish();
		}

		allocating.Increment();
		return res;
	}

	void UploadHeap::EndSuballocate() {
		allocating.Decrement();
	}

	void UploadHeap::AddBufferCopy(const void* data, int size, ID3D12Resource* bufferDst, D3D12_RESOURCE_STATES state) {
		UINT8* pixels = BeginSuballocate(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		memcpy(pixels, data, size);
		EndSuballocate();

		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_BufferCopies.push_back({
				bufferDst,
				(UINT64)(pixels - BasePtr()),
				size,
				state
			});

			D3D12_RESOURCE_BARRIER RBDesc = {};
			RBDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			RBDesc.Transition.pResource = bufferDst;
			RBDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			RBDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			RBDesc.Transition.StateAfter = state;
			m_ToBarrierIntoShaderResource.push_back(RBDesc);
		}
	}

	void UploadHeap::AddCopy(CD3DX12_TEXTURE_COPY_LOCATION src, CD3DX12_TEXTURE_COPY_LOCATION dst) {
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_TextureCopies.push_back({ src, dst });
	}

	void UploadHeap::AddBarrier(ID3D12Resource* res) {
		std::unique_lock<std::mutex> lock(m_Mutex);

		D3D12_RESOURCE_BARRIER RBDesc = {};
		RBDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		RBDesc.Transition.pResource = res;
		RBDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		RBDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		RBDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		m_ToBarrierIntoShaderResource.push_back(RBDesc);
	}

	void UploadHeap::FlushAndFinish() {
		flushing.Wait();
		flushing.Increment();
		allocating.Wait();

		std::unique_lock<std::mutex> lock(m_Mutex);

		for (TextureCopy c : m_TextureCopies) {
			m_CommandList->CopyTextureRegion(&c.dst, 0, 0, 0, &c.src, NULL);
		}
		m_TextureCopies.clear();

		for (BufferCopy c : m_BufferCopies) {
			D3D12_RESOURCE_BARRIER RBDesc = {};
			RBDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			RBDesc.Transition.pResource = c.bufferDst.Get();
			RBDesc.Transition.StateBefore = c.state;
			RBDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			m_CommandList->ResourceBarrier(1, &RBDesc);

			m_CommandList->CopyBufferRegion(c.bufferDst.Get(), 0, m_UploadHeap.Get(), c.offset, c.size);
		}
		m_BufferCopies.clear();

		if (!m_ToBarrierIntoShaderResource.empty()) {
			m_CommandList->ResourceBarrier(static_cast<UINT>(m_ToBarrierIntoShaderResource.size()), m_ToBarrierIntoShaderResource.data());
			m_ToBarrierIntoShaderResource.clear();
		}

		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* commandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, commandLists);

		m_Device->GPUFlush();
		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

		m_DataCur = m_DataBegin;
		flushing.Decrement();
	}
}