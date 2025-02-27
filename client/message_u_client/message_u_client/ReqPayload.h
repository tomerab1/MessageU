#pragma once

#include <vector>
#include <cstdint>

class ReqPayload {
public:
	using bytes_t = std::vector<uint8_t>;

	virtual bytes_t toBytes() = 0;
	virtual uint32_t getSize() = 0;
};