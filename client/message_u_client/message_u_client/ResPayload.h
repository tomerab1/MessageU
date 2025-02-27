#pragma once

#include <memory>
#include <vector>

enum class ResponseCodes : uint16_t;

class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	static payload_t fromBytes(const bytes_t& bytes, ResponseCodes code);
};