#pragma once

class MessageBuffer {
public:
	MessageBuffer(size_t len)
		: m_dynamic(len > STATIC_LEN ? len : 0),
		m_ptr(len > STATIC_LEN ? m_dynamic.data() : m_static) {
	}

	char* Data() { return m_ptr; }
private:
	static const size_t STATIC_LEN = 256;
	std::vector<char> m_dynamic;
	char m_static[STATIC_LEN] = { '0' };
	char* m_ptr;
};

std::string format(const char* format, ...);

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