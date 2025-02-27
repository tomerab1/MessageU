#include "Request.h"
#include "Config.h"
#include "Utils.h"
#include "Payload.h"

Request::Header::Header(id_t id, char version, uint16_t code, uint32_t payloadSz)
	: id{std::move(id)}, version{version}, code{code}, payloadSz{payloadSz}
{
}

Request::bytes_t Request::Header::toBytes()
{
	bytes_t bytes(Config::HEADER_BYTES_SZ, 0);
	size_t offset{ 0 };

	std::copy(id.begin(), id.end(), bytes.begin());
	offset += id.size();

	Utils::serializeTrivialType(bytes, offset, version);
	Utils::serializeTrivialType(bytes, offset, code);
	Utils::serializeTrivialType(bytes, offset, payloadSz);

	return bytes;
}

Request::Request(Request::id_t id, uint16_t code, payload_t payload)
	: m_payload{std::move(payload)}, m_header{id, Config::VERSION, code, payload->getSize()}
{
}

Request::bytes_t Request::toBytes()
{
}

Request::~Request()
{
}
