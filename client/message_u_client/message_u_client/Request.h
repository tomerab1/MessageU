#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "Config.h"

// Forward declaration of the request payload
class ReqPayload;

// Enum for the different request codes
enum class RequestCodes: uint16_t {
	REGISTER = 600,
	USRS_LIST = 601,
	GET_PUB_KEY = 602,
	SEND_MSG = 603,
	POLL_MSGS = 604,
};

// Enum for the different message types
enum class MessageTypes : uint8_t {
	GET_SYM_KEY = 1,
	SEND_SYM_KEY = 2,
	SEND_TXT = 3,
	SEND_FILE = 4,
};

// This class wraps the request header and payload
class Request {
public:
	// Type aliases
	using payload_t = std::unique_ptr<ReqPayload>;
	using bytes_t = std::vector<uint8_t>;
	
	struct Header {
		std::string id;
		char version;
		RequestCodes code;
		uint32_t payloadSz;
	
		Header(const std::string& id, char version, RequestCodes code, uint32_t payloadSz);
		
		// Converts a header to bytes
		bytes_t toBytes();
	};

	explicit Request(const std::string& id, RequestCodes code, payload_t payload);

	// Converts a request object to bytes
	bytes_t toBytes();

	// Gets the request code
	RequestCodes getCode();
	
	~Request();

private:
	Header m_header;
	payload_t m_payload;
};