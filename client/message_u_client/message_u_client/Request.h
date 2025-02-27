#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "Config.h"

class ReqPayload;

enum class RequestCodes: uint16_t {
	REGISTER = 600,
};

class Request {
public:
	using payload_t = std::unique_ptr<ReqPayload>;
	using bytes_t = std::vector<uint8_t>;
	using id_t = std::array<uint8_t, Config::CLIENT_ID_SZ>;
	
	struct Header {
		id_t id;
		char version;
		RequestCodes code;
		uint32_t payloadSz;
	
		Header(id_t id, char version, RequestCodes code, uint32_t payloadSz);
		bytes_t toBytes();
	};

	explicit Request(id_t id, RequestCodes code, payload_t payload);

	bytes_t toBytes();
	
	~Request();

private:
	Header m_header;
	payload_t m_payload;
};