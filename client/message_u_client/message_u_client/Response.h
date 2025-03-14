#pragma once

#include <cstdint>
#include <vector>
#include <memory>

// Enum for the different response codes
enum class ResponseCodes : uint16_t {
	REG_OK = 2100,
	USRS_LIST = 2101,
	PUB_KEY = 2102,
	MSG_SEND = 2103,
	POLL_MSGS = 2104,
	ERR = 9000,
};

// Fowrad declaration of the response payload
class ResPayload;

// This class wraps the response header and payload     
class Response {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	struct Header {
		uint8_t version;
		ResponseCodes code;
		uint32_t payloadSz;

		// Static method to create a header from bytes
		static Header fromBytes(const bytes_t& bytes);
	};

	Response(const Header& header, const bytes_t& payloadBytes);

	// Gets the header
	Header& getHeader();

	// Gets the payload
	ResPayload& getPayload();

	~Response();

private:
	Header m_header;
	payload_t m_payload;
};