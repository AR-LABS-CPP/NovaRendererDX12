#pragma once

namespace Nova {
	class UploadHeap {
		Sync allocating, flushing;

		struct TextureCopy {
			CD3DX12_TEXTURE_COPY_LOCATION src, dst;
		};
		std::vector<TextureCopy> m_TextureCopies;

		struct BufferCopy {
			Microsoft::WRL::ComPtr<ID3D12Resource> bufferDst;
			UINT64 offset;
			int size;
			D3D12_RESOURCE_STATES state;
		};

		std::vector<BufferCopy> m_BufferCopies;
		std::vector<D3D12_RESOURCE_BARRIER> m_ToBarrierIntoShaderResource;
		std::mutex m_Mutex;

	public:
		void Create(DXDevice* pDevice, SIZE_T uSize);
		void Destroy();

		UINT8* Suballocate(SIZE_T uSize, UINT64 uAlign);
		UINT8* BeginSuballocate(SIZE_T uSize, UINT64 uAlign);
		UINT8* BasePtr() { return m_DataBegin; }

		const Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource() const {
			return m_UploadHeap;
		}
		const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const {
			return m_CommandList;
		}
		const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() const {
			return m_CommandQueue;
		}

		void EndSuballocate();
		void AddBufferCopy(const void* data, int size, ID3D12Resource* bufferDst, D3D12_RESOURCE_STATES state);
		void AddCopy(CD3DX12_TEXTURE_COPY_LOCATION src, CD3DX12_TEXTURE_COPY_LOCATION dst);
		void AddBarrier(ID3D12Resource* res);
		void FlushAndFinish();

	private:
		DXDevice* m_Device;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHeap;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;

		UINT8* m_DataCur = nullptr;
		UINT8* m_DataEnd = nullptr;
		UINT8* m_DataBegin = nullptr;
	};
}