#pragma once

namespace Nova {
	class DXResourceView {
	public:
		uint32_t GetSize() const {
			return m_Size;
		}

		uint32_t GetDescriptorSize() const {
			return m_DescriptorSize;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPU(uint32_t idx = 0) const {
			D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor = m_CPUDescriptor;
			CPUDescriptor.ptr = idx * m_DescriptorSize;

			return CPUDescriptor;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPU(uint32_t idx = 0) const {
			D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor = m_GPUDescriptor;
			GPUDescriptor.ptr = idx * m_DescriptorSize;

			return GPUDescriptor;
		}

		void SetResourceView(
			uint32_t size,
			uint32_t dsvDescriptorSize,
			D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor,
			D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor
		) {
			m_Size = size;
			m_DescriptorSize = dsvDescriptorSize;
			m_CPUDescriptor = CPUDescriptor;
			m_GPUDescriptor = GPUDescriptor;
		}
	private:
		friend class DXStaticResourceView;
		friend class DXDynamicResourceView;

		uint32_t m_Size = 0;
		uint32_t m_DescriptorSize = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptor;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescriptor;
	};

	class RTV : public DXResourceView {};
	class DSV : public DXResourceView {};
	class CBV_SRV_UAV : public DXResourceView {};
	class Sampler : public DXResourceView {};

	class DXStaticResourceViewHeap {
	public:
		void Create(
			DXDevice* device,
			D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			uint32_t descriptorCount,
			bool forceCPUVisible = false
		);
		void Destroy();
		bool AllocDescriptor(uint32_t size, DXResourceView* prv);

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetHeap() const {
			return m_Heap;
		}
	private:
		uint32_t m_Index;
		uint32_t m_DescriptorCount;
		uint32_t m_DescriptorElementSize;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	};

	class DXResourceViewHeaps {
	public:
		void OnCreate(
			DXDevice* device,
			uint32_t cbvDescriptorCount,
			uint32_t srvDescriptorCount,
			uint32_t uavDescriptorCount,
			uint32_t dsvDescriptorCount,
			uint32_t rtvDescriptorCount,
			uint32_t samplerDescriptorCount
		);
		void Destroy();

		bool AllocateCBV_SRV_UAVDescriptor(uint32_t size, CBV_SRV_UAV* prv);
		bool AllocateDSVDescriptor(uint32_t size, DSV* prv);
		bool AllocateRTVDescriptor(uint32_t size, RTV* prv);
		bool AllocateSamplerDescriptor(uint32_t size, Sampler* prv);

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetDSVHeap() const {
			return m_DSVHeap.GetHeap();
		}

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetRTVHeap() const {
			return m_RTVHeap.GetHeap();
		}

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSamplerHeap() const {
			return m_SamplerHeap.GetHeap();
		}

		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetCBV_SRV_UAVHeap() const {
			return m_CBV_SRV_UAV_Heap.GetHeap();
		}
	private:
		DXStaticResourceViewHeap m_DSVHeap;
		DXStaticResourceViewHeap m_RTVHeap;
		DXStaticResourceViewHeap m_SamplerHeap;
		DXStaticResourceViewHeap m_CBV_SRV_UAV_Heap;
	};
}