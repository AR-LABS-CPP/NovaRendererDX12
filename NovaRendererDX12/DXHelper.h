#pragma once

namespace Nova {
	void SetViewportAndScissor(
		ID3D12GraphicsCommandList* commandList,
		uint32_t topLeftX,
		uint32_t topLeftY,
		uint32_t width,
		uint32_t height
	);

	void SetName(ID3D12Object* obj, const char* name);

	void SetName(ID3D12Object* obj, const std::string& name);
}