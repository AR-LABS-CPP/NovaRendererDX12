#pragma once

namespace Nova {
	class DXCommandList {
	public:
		void Create(
			DXDevice* device,
			uint32_t numberOfBackBuffers,
			uint32_t commandListsPerBackBuffer,
			const D3D12_COMMAND_QUEUE_DESC& queueDesc
		);
		void Destroy();
		void BeginFrame();

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetNewCommandList();
		const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetAllocator() const {
			return m_CurrentFrame->m_CommandAllocator;
		}
	private:
		struct CommandBuffersPerFrame {
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
			std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>> m_CommandLists;
			uint32_t m_UsedCls;
		};

		std::vector<CommandBuffersPerFrame> m_CommandBuffers;
		CommandBuffersPerFrame* m_CurrentFrame = nullptr;

		uint32_t m_FrameIndex;
		uint32_t m_NumberOfAllocators;
		uint32_t m_CommandListsPerBackBuffer;
	};
}