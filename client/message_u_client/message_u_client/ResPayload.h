#pragma once

#include <memory>
#include <vector>

class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	virtual payload_t fromBytes(const bytes_t& bytes) = 0;
};