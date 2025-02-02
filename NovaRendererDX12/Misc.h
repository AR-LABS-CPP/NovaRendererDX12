#pragma once

#include <vector>
#include <string>

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