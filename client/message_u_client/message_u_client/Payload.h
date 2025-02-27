#pragma once

#include <vector>
#include <cstdint>

class Payload {
public:
	using bytes_t = std::vector<uint8_t>;

	virtual bytes_t toBytes() = 0;
	virtual size_t getSize() = 0;
};