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
		return std::make_unique<ErrorPayload>();
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

void RegistrationResPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
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

void UsersListResPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
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

void PublicKeyResPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
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

const MessageSentResPayload::MsgEntry& MessageSentResPayload::getMessage() const
{
	return m_entry;
}

void MessageSentResPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
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

const std::vector<PollMessageResPayload::MessageEntry>& PollMessageResPayload::getMessages() const
{
	return m_msgs;
}

void PollMessageResPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
}

void ErrorPayload::accept(Visitor& visitor)
{
	visitor.visit(*this);
}

std::string ToStringVisitor::getString()
{
	return m_ss.str();
}

void ToStringVisitor::visit(const RegistrationResPayload& payload)
{
	m_ss << boost::algorithm::hex(payload.getUUID());
}

void ToStringVisitor::visit(const UsersListResPayload& payload)
{
	for (const auto& user : payload.getUsers()) {
		m_ss << boost::algorithm::hex(user.id) << '\t' << user.name << '\n';
	}
}

void ToStringVisitor::visit(const PublicKeyResPayload& payload)
{
	m_ss << boost::algorithm::hex(payload.getPubKeyEntry().id) << '\t' << payload.getPubKeyEntry().pubKey << '\n';
}

void ToStringVisitor::visit(const MessageSentResPayload& payload)
{
	m_ss << boost::algorithm::hex(payload.getMessage().targetId) << '\t' << payload.getMessage().msgId;
}

void ToStringVisitor::visit(const PollMessageResPayload& payload)
{
	auto messages = payload.getMessages();
	for (size_t i = 0; i < messages.size(); i++) {
		m_ss << (i + 1) << ' ' << messages[i].content << "\n\n";
	}
}

void ToStringVisitor::visit(const ErrorPayload& payload)
{
	m_ss << std::string("Server responded with a generic error");
}
