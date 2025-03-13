#include "ReqPayload.h"
#include "Request.h"
#include "Config.h"
#include "Utils.h"

#include <boost/endian/conversion.hpp>

RegisterReqPayload::RegisterReqPayload(const name_t& name, const pub_key_t& pubKey)
	: m_name{ name }, m_pubKey{ pubKey }
{
}

RegisterReqPayload::bytes_t RegisterReqPayload::toBytes()
{
	bytes_t bytes;
	bytes.resize(getSize());

	std::copy(m_name.begin(), m_name.end(), bytes.begin());
	std::copy(m_pubKey.begin(), m_pubKey.end(), bytes.begin() + Config::NAME_MAX_SZ);

	return bytes;
}

uint32_t RegisterReqPayload::getSize()
{
	return Config::NAME_MAX_SZ + Config::PUB_KEY_SZ;
}


UsersListReqPayload::bytes_t UsersListReqPayload::toBytes()
{
	return bytes_t();
}

uint32_t UsersListReqPayload::getSize()
{
	return 0;
}

GetPublicKeyReqPayload::GetPublicKeyReqPayload(const std::string& targetId)
	: m_targetId{ targetId }
{
}

GetPublicKeyReqPayload::bytes_t GetPublicKeyReqPayload::toBytes()
{
	bytes_t bytes;
	bytes.resize(getSize());

	std::copy(m_targetId.begin(), m_targetId.end(), bytes.begin());

	return bytes;
}

uint32_t GetPublicKeyReqPayload::getSize()
{
	return Config::CLIENT_ID_SZ;
}


SendMessageReqPayload::SendMessageReqPayload(const std::string& targetId, MessageTypes type, uint32_t msgSz, const std::string& msg)
	: m_targetId{ targetId }, m_type{ type }, m_msgSz{ msgSz }, m_msg{ msg }
{
}

SendMessageReqPayload::bytes_t SendMessageReqPayload::toBytes()
{
	bytes_t bytes;
	size_t offset{ 0 };
	bytes.resize(getSize());

	std::copy(m_targetId.begin(), m_targetId.end(), bytes.begin());
	offset += Config::CLIENT_ID_SZ;

	Utils::serializeTrivialType(bytes, offset, Utils::EnumToUint8(m_type));
	Utils::serializeTrivialType(bytes, offset, m_msgSz);

	std::copy(m_msg.begin(), m_msg.end(), bytes.begin() + offset);

	return bytes;
}

uint32_t SendMessageReqPayload::getSize()
{
	return m_msgSz + sizeof(MessageTypes) + Config::CLIENT_ID_SZ + sizeof(m_msgSz);
}

PollMessagesReqPayload::bytes_t PollMessagesReqPayload::toBytes()
{
	return bytes_t();
}

uint32_t PollMessagesReqPayload::getSize()
{
	return 0;
}

SendFileReqPayload::SendFileReqPayload(const std::string& targetId, MessageTypes type, uint32_t fileSz, const std::string& filePath)
	: SendMessageReqPayload{ targetId, type, fileSz, filePath }, m_file{filePath}
{
	if (!m_file.is_open()) {
		throw std::runtime_error("Error: Could not open '" + filePath + "'");
	}
}

SendFileReqPayload::bytes_t SendFileReqPayload::toBytes()
{
	bytes_t buff;
	buff.resize(Config::CHUNK_SZ);

	m_file.read(reinterpret_cast<char*>(buff.data()), Config::CHUNK_SZ);
	auto bytesRead = m_file.gcount();

	if (!bytesRead) {
		return bytes_t();
	}

	return buff;
}
