#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

static constexpr size_t CLIENT_ID_SZ = 16;

class Payload;

class Request {
public:
	using payload_t = std::unique_ptr<Payload>;
	using bytes_t = std::vector<uint8_t>;
	using id_t = std::array<uint8_t, CLIENT_ID_SZ>;
	
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
	const Header m_header;
	payload_t m_payload;
};