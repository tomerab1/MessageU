#include "Response.h"
#include "ResPayload.h"
#include "Utils.h"

Response::Header Response::Header::fromBytes(const bytes_t& bytes)
{
    size_t offset{ 0 };

    auto version = Utils::deserializeTrivialType<uint8_t>(bytes, offset);
    auto code = Utils::deserializeTrivialType<uint16_t>(bytes, offset);
    auto payloadSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);

    return {version, static_cast<ResponseCodes>(code), payloadSz};
}

Response::Response(const Header& header, const bytes_t& payloadBytes)
    : m_header{header}
{
    m_payload = ResPayload::fromBytes(payloadBytes, m_header.code);
}

Response::Header& Response::getHeader()
{
    return m_header;
}

ResPayload& Response::getPayload()
{
    return *m_payload;
}

Response::~Response() = default;