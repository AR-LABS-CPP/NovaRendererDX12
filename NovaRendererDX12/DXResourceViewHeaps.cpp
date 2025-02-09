#include "stdafx.h"
#include "DXError.h"
#include "DXHelper.h"
#include "DXDevice.h"
#include "DXResourceViewHeaps.h"

namespace Nova {
	void DXStaticResourceViewHeap::Create(
		DXDevice* device,
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		uint32_t descriptorCount,
		bool forceCPUVisible
	) {
		m_DescriptorCount = descriptorCount;
		m_Index = 0;
		m_DescriptorElementSize = device->GetDevice()->GetDescriptorHandleIncrementSize(heapType);

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = descriptorCount;
		heapDesc.Type = heapType;
		heapDesc.Flags = (
			(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) || 
			(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV))
			? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (forceCPUVisible) {
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		}

		heapDesc.NodeMask = 0;
		ThrowIfFailed(device->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));
		SetName(m_Heap.Get(), "DX12StaticHeap");
	}

	void DXStaticResourceViewHeap::Destroy() {
		m_Heap.Reset();
	}
	
	bool DXStaticResourceViewHeap::AllocDescriptor(uint32_t size, DXResourceView* prv) {
		if ((m_Index + size) > m_DescriptorCount) {
			assert(!"DXStaticResourceViewHeap Heap ran out of memory, boom boom!");
			return false;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CPUView = m_Heap->GetCPUDescriptorHandleForHeapStart();
		CPUView.ptr += m_Index * m_DescriptorElementSize;

		D3D12_GPU_DESCRIPTOR_HANDLE GPUView = {};
		if (m_Heap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
			GPUView = m_Heap->GetGPUDescriptorHandleForHeapStart();
			GPUView.ptr += m_Index * m_DescriptorElementSize;
		}

		m_Index += size;
		prv->SetResourceView(size, m_DescriptorElementSize, CPUView, GPUView);
		
		return true;
	}

	void DXResourceViewHeaps::OnCreate(
		DXDevice* device,
		uint32_t cbvDescriptorCount,
		uint32_t srvDescriptorCount,
		uint32_t uavDescriptorCount,
		uint32_t dsvDescriptorCount,
		uint32_t rtvDescriptorCount,
		uint32_t samplerDescriptorCount
	) {
		m_DSVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvDescriptorCount);
		m_RTVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvDescriptorCount);
		m_SamplerHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerDescriptorCount);
		m_CBV_SRV_UAV_Heap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvDescriptorCount + srvDescriptorCount + uavDescriptorCount);
	}
	
	void DXResourceViewHeaps::Destroy() {
		m_DSVHeap.Destroy();
		m_RTVHeap.Destroy();
		m_SamplerHeap.Destroy();
		m_CBV_SRV_UAV_Heap.Destroy();
	}

	bool DXResourceViewHeaps::AllocateCBV_SRV_UAVDescriptor(uint32_t size, CBV_SRV_UAV* prv) {
		return m_CBV_SRV_UAV_Heap.AllocDescriptor(size, prv);
	}
	
	bool DXResourceViewHeaps::AllocateDSVDescriptor(uint32_t size, DSV* prv) {
		return m_DSVHeap.AllocDescriptor(size, prv);
	}
	
	bool DXResourceViewHeaps::AllocateRTVDescriptor(uint32_t size, RTV* prv) {
		return m_RTVHeap.AllocDescriptor(size, prv);
	}
	
	bool DXResourceViewHeaps::AllocateSamplerDescriptor(uint32_t size, Sampler* prv) {
		return m_SamplerHeap.AllocDescriptor(size, prv);
	}
}