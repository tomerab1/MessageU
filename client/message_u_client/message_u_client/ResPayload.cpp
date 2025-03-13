#include "ResPayload.h"
#include "Response.h"
#include "Request.h"
#include "Client.h"
#include "Utils.h"
#include "Config.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

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
	}

	throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
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

		auto pos = std::find(curr.name.begin(), curr.name.end(), '\0');
		if (pos != curr.name.end()) {
			curr.name.resize(std::distance(curr.name.begin(), pos));
		}
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
	std::copy(bytes.begin() + Config::CLIENT_ID_SZ, bytes.end(), m_entry.pubKey.begin());
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

ToStringVisitor::ToStringVisitor(ClientState& state)
	: m_state{state}
{
}

std::string ToStringVisitor::getString()
{
	std::string result = m_ss.str();
	m_ss.clear();
	return result;
}

void ToStringVisitor::visit(const RegistrationResPayload& payload)
{
	m_ss << boost::algorithm::hex(payload.getUUID());
}

void ToStringVisitor::visit(const UsersListResPayload& payload)
{
	if (payload.getUsers().empty()) {
		m_ss << "There are no other registered clients at the moment";
		return;
	}

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
		m_ss << "From: " << m_state.getNameByUUID(messages[i].senderId) << '\n';
		m_ss << "Content:\n";
		
		switch (messages[i].msgType) {
		case MessageTypes::SEND_TXT: {
			auto username = m_state.getNameByUUID(messages[i].senderId);
			auto symKey = m_state.getSymKey(username);

			if (!symKey) {
				m_ss << "can’t decrypt message";
				break;
			}

			auto msg = messages[i].content;
			AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.value().c_str()), symKey.value().size());

			m_ss << aes.decrypt(msg.c_str(), msg.size());
			break;
		}
		case MessageTypes::GET_SYM_KEY:
			m_ss << "Request for symmetric key";
			break;
		case MessageTypes::SEND_SYM_KEY:
			m_ss << "Symmetric key received";
			break;
		case MessageTypes::SEND_FILE:
			m_ss << "FILE_PLACEHOLDER";
			break;
		default:
			break;
		}

		m_ss << "\n-----<EOM>-----\n\n";
	}
}

void ToStringVisitor::visit(const ErrorPayload& payload)
{
	m_ss << std::string("Server responded with a generic error");
}

ClientStateVisitor::ClientStateVisitor(ClientState& state)
	: m_state{state}
{
}

void ClientStateVisitor::visit(const UsersListResPayload& payload)
{
	for (const auto& entry : payload.getUsers()) {
		m_state.addClient(entry.name, entry.id);
	}
}

void ClientStateVisitor::visit(const PublicKeyResPayload& payload)
{
	auto entry = payload.getPubKeyEntry();
	auto name = m_state.getNameByUUID(entry.id);
	m_state.setPubKey(name, entry.pubKey);
}

void ClientStateVisitor::visit(const PollMessageResPayload& payload)
{
	auto messages = payload.getMessages();
	for (size_t i = 0; i < messages.size(); i++) {
		switch (messages[i].msgType) {
		case MessageTypes::SEND_SYM_KEY: {
			auto privKey = m_state.getPrivKey();
			auto rsaprive = RSAPrivateWrapper(privKey);
			auto username = m_state.getNameByUUID(messages[i].senderId);

			m_state.setSymKey(username, rsaprive.decrypt(messages[i].content));
		}
		break;
		default:
			break;
		}
	}
}

void ClientStateVisitor::visit(const RegistrationResPayload& payload)
{
	throw std::logic_error("Error: Unreachable");
}

void ClientStateVisitor::visit(const MessageSentResPayload& payload)
{
	throw std::logic_error("Error: Unreachable");
}

void ClientStateVisitor::visit(const ErrorPayload& payload)
{
}
