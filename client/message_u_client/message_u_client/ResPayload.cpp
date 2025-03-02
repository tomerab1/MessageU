#include "ResPayload.h"
#include "Response.h"
#include "Request.h"
#include "Utils.h"
#include "Config.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <boost/algorithm/hex.hpp>

ResPayload::payload_t ResPayload::fromBytes(const bytes_t& bytes, ResponseCodes code)
{
	switch (code)
	{
	case ResponseCodes::REG_OK:
		return std::make_unique<RegistrationResPayload>(bytes);
	case ResponseCodes::USRS_LIST:
		return std::make_unique<UsersListResPayload>(bytes);
	case ResponseCodes::PUB_KEY:
		return std::make_unique<PublicKeyResPayload>(bytes);
	case ResponseCodes::MSG_SEND:
		return std::make_unique<MessageSentResPayload>(bytes);
	case ResponseCodes::POLL_MSGS:
		return std::make_unique<PollMessageResPayload>(bytes);
	case ResponseCodes::ERR: 
		break;
	default:
		throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
	}
}

RegistrationResPayload::RegistrationResPayload(const bytes_t& bytes)
	: m_uuid{}
{
	ptrdiff_t diff = bytes.end() - bytes.begin();
	if (diff == 0 || diff > Config::CLIENT_ID_SZ) {
		throw std::runtime_error("Error: Received an invalid user id");
	}

	m_uuid.resize(diff);
	std::copy(bytes.begin(), bytes.end(), m_uuid.begin());
}

const std::string& RegistrationResPayload::getUUID() const
{
	return m_uuid;
}

std::string RegistrationResPayload::toString() const
{
	std::stringstream ss;

	ss << boost::algorithm::hex(getUUID());

	return ss.str();
}


UsersListResPayload::UsersListResPayload(const bytes_t& bytes)
{
	if (bytes.empty()) {
		throw std::runtime_error("Error: UsersListResPayload payload is empty");
	}

	size_t numUsers = bytes.size() / (Config::CLIENT_ID_SZ + Config::NAME_MAX_SZ);
	size_t offset{ 0 };
	m_users.resize(numUsers);

	for (size_t i = 0; i < numUsers; i++) {
		UserEntry& curr = m_users[i];
		curr.id.resize(Config::CLIENT_ID_SZ);
		curr.name.resize(Config::NAME_MAX_SZ);

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::CLIENT_ID_SZ, curr.id.begin());
		offset += Config::CLIENT_ID_SZ;

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::NAME_MAX_SZ, curr.name.begin());
		offset += Config::NAME_MAX_SZ;

		curr.name.shrink_to_fit();
	}
}

std::string UsersListResPayload::toString() const
{
	std::stringstream ss;

	for (const auto& user : getUsers()) {
		ss << boost::algorithm::hex(user.id) << '\t' << user.name << '\n';
	}

	return ss.str();
}

const std::vector<UsersListResPayload::UserEntry>& UsersListResPayload::getUsers() const
{
	return m_users;
}

PublicKeyResPayload::PublicKeyResPayload(const bytes_t& bytes)
{
	if (bytes.empty()) {
		throw std::runtime_error("Error: PublicKeyPayload payload is empty");
	}

	m_entry.id.resize(Config::CLIENT_ID_SZ);
	m_entry.pubKey.resize(Config::PUB_KEY_SZ);

	std::copy(bytes.begin(), bytes.begin() + Config::CLIENT_ID_SZ, m_entry.id.begin());
	std::copy(bytes.begin(), bytes.begin() + Config::PUB_KEY_SZ, m_entry.pubKey.begin());
}

std::string PublicKeyResPayload::toString() const
{

	std::stringstream ss;

	ss << boost::algorithm::hex(getPubKeyEntry().id) << '\t' << getPubKeyEntry().pubKey << '\n';

	return ss.str();
}

const PublicKeyResPayload::PublicKeyEntry& PublicKeyResPayload::getPubKeyEntry() const
{
	return m_entry;
}

MessageSentResPayload::MessageSentResPayload(const bytes_t& bytes)
{
	m_entry.targetId.resize(Config::CLIENT_ID_SZ);
	std::copy(bytes.begin(), bytes.begin() + Config::CLIENT_ID_SZ, m_entry.targetId.begin());
	
	size_t offset{ Config::CLIENT_ID_SZ };
	m_entry.msgId = Utils::deserializeTrivialType<uint32_t>(bytes, offset);
}

std::string MessageSentResPayload::toString() const
{
	std::stringstream ss;

	ss << boost::algorithm::hex(m_entry.targetId) << '\t' << m_entry.msgId;

	return ss.str();
}

PollMessageResPayload::PollMessageResPayload(const bytes_t& bytes)
{
	size_t offset{ 0 };
	while (offset < bytes.size()) {
		MessageEntry msg;
		msg.senderId.resize(Config::CLIENT_ID_SZ);

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::CLIENT_ID_SZ, msg.senderId.begin());
		offset += Config::CLIENT_ID_SZ;

		msg.msgId = Utils::deserializeTrivialType<uint32_t>(bytes, offset);
		msg.msgType = MessageTypes(Utils::deserializeTrivialType<uint8_t>(bytes, offset));
		msg.contentSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);

		msg.content.resize(msg.contentSz);
		std::copy(bytes.begin() + offset, bytes.begin() + offset + msg.contentSz, msg.content.begin());
		offset += msg.contentSz;

		m_msgs.push_back(msg);
	}
}

std::string PollMessageResPayload::toString() const
{
	std::stringstream ss;
	
	for (size_t i = 0; i < m_msgs.size(); i++) {
		ss << (i + 1) << ' ' << m_msgs[i].content << "\n\n";
	}

	return ss.str();
}
