#include "Request.h"
#include "Config.h"
#include "Utils.h"
#include "ReqPayload.h"

#include <iostream>

Request::Header::Header(const std::string& id, char version, RequestCodes code, uint32_t payloadSz)
	: id{id}, version{version}, code{code}, payloadSz{payloadSz}
{
}

Request::bytes_t Request::Header::toBytes()
{
	bytes_t bytes;
	size_t offset{ 0 };
	
	// Resize the bytes vector to the size of the header
	bytes.resize(Config::HEADER_BYTES_SZ);
	// Copy the client id
	std::copy(id.begin(), id.end(), bytes.begin());
	offset += Config::CLIENT_ID_SZ;

	// Serialize the version, request code and payload size
	Utils::serializeTrivialType(bytes, offset, version);
	Utils::serializeTrivialType(bytes, offset, Utils::EnumToUint16(code));
	Utils::serializeTrivialType(bytes, offset, payloadSz);

	return bytes;
}

Request::Request(const std::string& id, RequestCodes code, payload_t payload)
	: m_payload{std::move(payload)}, m_header{id, Config::VERSION, code, payload->getSize()}
{
}

Request::bytes_t Request::toBytes()
{
	// Get the bytes of the header and the payload
	auto headerBytes = m_header.toBytes();
	auto payloadBytes = m_payload->toBytes();
	bytes_t bytes;
	size_t offset{ 0 };

	// Resize the bytes vector to be of their combined size, copy the header and then the payload
	bytes.resize(headerBytes.size() + payloadBytes.size());
	std::copy(headerBytes.begin(), headerBytes.end(), bytes.begin());
	offset += headerBytes.size();

	std::copy(payloadBytes.begin(), payloadBytes.end(), bytes.begin() + offset);

	return bytes;
}

RequestCodes Request::getCode()
{
	return m_header.code;
}

Request::~Request()
{
}
