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

Response::Response(const bytes_t& bytes)
{
    this->m_header = Header::fromBytes(bytes);
    this->m_payload = nullptr;
}

const Response::Header& Response::getHeader()
{
    return m_header;
}

const ResPayload& Response::getPayload()
{
    return *m_payload;
}
