#pragma once

#include <cstdint>
#include <vector>
#include <memory>

class ResPayload;

enum class ResponseCodes : uint16_t {
	REG_OK = 2100
};

class Response {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	struct Header {
		uint8_t version;
		ResponseCodes code;
		uint32_t payloadSz;
	
		static Header fromBytes(const bytes_t& bytes);
	};

	static Response fromBytes(const bytes_t& bytes);

	const Header& getHeader();
	const ResPayload& getPayload();


private:
	Header m_header;
	payload_t m_payload;
};