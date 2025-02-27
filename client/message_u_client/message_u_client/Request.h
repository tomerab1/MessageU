#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "Config.h"

class Payload;

class Request {
public:
	using payload_t = std::unique_ptr<Payload>;
	using bytes_t = std::vector<uint8_t>;
	using id_t = std::array<uint8_t, Config::CLIENT_ID_SZ>;
	
	struct Header {
		id_t id;
		char version;
		uint16_t code;
		uint32_t payloadSz;
	
		Header(id_t id, char version, uint16_t code, uint32_t payloadSz);
		bytes_t toBytes();
	};

	explicit Request(id_t id, uint16_t code, payload_t payload);

	bytes_t toBytes();
	
	~Request();

private:
	Header m_header;
	payload_t m_payload;
};