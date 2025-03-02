#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "Config.h"

class ReqPayload;

enum class RequestCodes: uint16_t {
	REGISTER = 600,
	USRS_LIST = 601,
	GET_PUB_KEY = 602,
	SEND_MSG = 603,
	POLL_MSGS = 604,
};

enum class MessageTypes : uint8_t {
	GET_SYM_KEY = 1,
	SEND_SYM_KEY = 2,
	SEND_TXT = 3,
	SEND_FILE = 4,
};

class Request {
public:
	using payload_t = std::unique_ptr<ReqPayload>;
	using bytes_t = std::vector<uint8_t>;
	
	struct Header {
		std::string id;
		char version;
		RequestCodes code;
		uint32_t payloadSz;
	
		Header(const std::string& id, char version, RequestCodes code, uint32_t payloadSz);
		bytes_t toBytes();
	};

	explicit Request(const std::string& id, RequestCodes code, payload_t payload);

	bytes_t toBytes();
	
	~Request();

private:
	Header m_header;
	payload_t m_payload;
};