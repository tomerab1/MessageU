#include "RegisterReqPayload.h"

#include <boost/endian/conversion.hpp>

RegisterReqPayload::RegisterReqPayload(const name_t& name, const pub_key_t& pubKey)
    : m_name{name}, m_pubKey{pubKey}
{
}

RegisterReqPayload::bytes_t RegisterReqPayload::toBytes()
{
    bytes_t bytes(Config::NAME_MAX_SZ + Config::PUB_KEY_SZ, 0);

    std::copy(m_name.begin(), m_name.end(), bytes.begin());
    std::copy(m_pubKey.begin(), m_pubKey.end(), bytes.begin() + Config::NAME_MAX_SZ);

    return bytes;
}

uint32_t RegisterReqPayload::getSize()
{
    return Config::NAME_MAX_SZ + Config::PUB_KEY_SZ;
}
