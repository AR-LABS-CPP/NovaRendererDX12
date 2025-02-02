#include "stdafx.h"
#include "DXError.h"
#include "DXHelper.h"
#include "DXDevice.h"

namespace Nova {
	void DXDevice::Create(
		const char* appName,
		const char* engine,
		bool validationEnabled,
		bool gpuValidationEnabled,
		HWND hWnd
	) {
		// Enable debug layer
		if (validationEnabled || gpuValidationEnabled) {
			Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;

			if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK) {
				debugController->EnableDebugLayer();
				debugController->SetEnableGPUBasedValidation(gpuValidationEnabled);
			}
		}

		// Init adapter
		{
			UINT factoryFlags = 0;

			if (validationEnabled || gpuValidationEnabled) {
				factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}

			Microsoft::WRL::ComPtr<IDXGIFactory> factory;
			Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

			ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)));

			if (factory->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK) {
				ThrowIfFailed(factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_Adapter)));
			}
			else {
				ThrowIfFailed(factory->EnumAdapters(0, m_Adapter.GetAddressOf()));
			}
		}

		// Create device
		ThrowIfFailed(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));

		if (validationEnabled || gpuValidationEnabled) {
			Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;

			if (m_Device->QueryInterface(IID_PPV_ARGS(&infoQueue)) == S_OK) {
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			}
		}

		SetName(m_Device.Get(), "device");

		// Check for min precision floating point support
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureDataOptions = {};
		ThrowIfFailed(m_Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS,
			&featureDataOptions,
			sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS)
		));
		m_fp16Supported = (featureDataOptions.MinPrecisionSupport
			& D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT) != 0;

		// Check for ray tracing support
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureDataOptions5 = {};
		ThrowIfFailed(m_Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS5,
			&featureDataOptions5,
			sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5)
		));
		m_rt10Supported = (featureDataOptions5.RaytracingTier
			& D3D12_RAYTRACING_TIER_1_0) != 0;
		m_rt11Supported = (featureDataOptions5.RaytracingTier
			& D3D12_RAYTRACING_TIER_1_1) != 0;

		// Check for variable rate shading support
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 featureDataOptions6 = {};
		ThrowIfFailed(m_Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS6,
			&featureDataOptions6,
			sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS6)
		));
		m_vrs1Supported = (featureDataOptions6.VariableShadingRateTier
			& D3D12_VARIABLE_SHADING_RATE_TIER_1) != 0;
		m_vrs2Supported = (featureDataOptions6.VariableShadingRateTier
			& D3D12_VARIABLE_SHADING_RATE_TIER_2) != 0;

		// Check for Barycentrics support
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureDataOptions3 = {};
		ThrowIfFailed(m_Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS3,
			&featureDataOptions3,
			sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS3)
		));
		m_barycentricsSupported = featureDataOptions3.BarycentricsSupported;

		// Create direct and compute queues
		{
			D3D12_COMMAND_QUEUE_DESC directQueueInfo = {};
			directQueueInfo.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			directQueueInfo.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			directQueueInfo.NodeMask = 0;

			ThrowIfFailed(m_Device->CreateCommandQueue(&directQueueInfo,
				IID_PPV_ARGS(&m_DirectQueue)));
			SetName(m_DirectQueue.Get(), "DirectQueue");

			D3D12_COMMAND_QUEUE_DESC computeQueueInfo = {};
			computeQueueInfo.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			computeQueueInfo.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			computeQueueInfo.NodeMask = 0;

			ThrowIfFailed(m_Device->CreateCommandQueue(&computeQueueInfo,
				IID_PPV_ARGS(&m_ComputeQueue)));
			SetName(m_ComputeQueue.Get(), "ComputeQueue");
		}
	}

	void DXDevice::Destroy() {
		m_ComputeQueue.Reset();
		m_DirectQueue.Reset();
		m_Adapter.Reset();
		m_Device.Reset();

#ifdef  _DEBUG
		// Useful for checking memory leaks and/or unwanted things
		{
			Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug1;

			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug1)))) {
				dxgiDebug1->ReportLiveObjects(
					DXGI_DEBUG_ALL,
					DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
				);
			}
		}
#endif //  _DEBUG
	}

	void DXDevice::GetDeviceInfo(std::string* deviceName, std::string* driverVersion) {
		DXGI_ADAPTER_DESC adapterDesc;
		m_Adapter->GetDesc(&adapterDesc);

		*deviceName = format("%S", adapterDesc.Description);
		*driverVersion = "Currently Unknown.";
	}

	void DXDevice::GPUFlush(D3D12_COMMAND_LIST_TYPE queueType) {
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue = (queueType == D3D12_COMMAND_LIST_TYPE_COMPUTE) ? GetComputeQueue() : GetGraphicsQueue();
		ThrowIfFailed(queue->Signal(fence.Get(), 1));

		HANDLE hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(1, hFenceEvent);

		WaitForSingleObject(hFenceEvent, INFINITE);
		CloseHandle(hFenceEvent);
	}

	void DXDevice::GPUFlush() {
		GPUFlush(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		GPUFlush(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
}
