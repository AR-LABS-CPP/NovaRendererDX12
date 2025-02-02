#include "stdafx.h"
#include "DXHelper.h"

namespace Nova {
	void SetViewportAndScissor(
		ID3D12GraphicsCommandList* commandList,
		uint32_t topLeftX,
		uint32_t topLeftY,
		uint32_t width,
		uint32_t height
	) {
		D3D12_VIEWPORT viewport = {
			static_cast<float>(topLeftX),
			static_cast<float>(topLeftY),
			static_cast<float>(width),
			static_cast<float>(height),
			0.0f,
			1.0f
		};

		D3D12_RECT rect = {
			(LONG)topLeftX,
			(LONG)topLeftY,
			(LONG)(topLeftX + width),
			(LONG)(topLeftY + height)
		};

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rect);
	}

	void SetName(ID3D12Object* obj, const char* name) {
		if (name != NULL) {
			SetName(obj, std::string(name));
		}
	}

	void SetName(ID3D12Object* obj, const std::string& name) {
		assert(obj != NULL);
		wchar_t nameBuffer[128];

		if (name.size() >= 128) {
			swprintf(nameBuffer, 128, L"%hs", name.substr(name.size() - 127, 127).c_str());
		}
		else {
			swprintf(nameBuffer, name.size() + 1, L"%S", name.c_str());
		}

		obj->SetName(nameBuffer);
	}
}

std::string format(const char* format, ...) {
	va_list args;
	va_start(args, format);

#ifndef _MSC_VER
	// Extra space for '\0'
	size_t size = std::snprintf(nullptr, 0, format, args) + 1;
	MessageBuffer buf(size);
	
	std::vsnprintf(buf.Data(), size, format, args);
	va_end(args);

	// We don't want the '\0' inside
	return std::string(buf.Data(), buf.Data() + size - 1); 
#else
	const size_t size = (size_t)_vscprintf(format, args) + 1;
	MessageBuffer buf(size);
	
	vsnprintf_s(buf.Data(), size, _TRUNCATE, format, args);
	va_end(args);
	
	return std::string(buf.Data(), buf.Data() + size - 1);
#endif
}