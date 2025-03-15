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
#include <fstream>
#include <filesystem>
#include <chrono>
#include <limits>
#include <boost/algorithm/hex.hpp>

ResPayload::payload_t ResPayload::fromBytes(const bytes_t& bytes, ResponseCodes code)
{
	// Create the appropriate payload object based on the response code
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

	// In case there is no match, throw a runtime error
	throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
}

RegistrationResPayload::RegistrationResPayload(const bytes_t& bytes)
	: m_uuid{}
{
	// Copy the UUID from the byte array
	m_uuid.resize(Config::CLIENT_ID_SZ);
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
	// Calculate the number of users in the list
	size_t numUsers = bytes.size() / (Config::CLIENT_ID_SZ + Config::NAME_MAX_SZ);
	size_t offset{ 0 };
	m_users.resize(numUsers);
	
	// Parse the byte array to extract the user entries
	for (size_t i = 0; i < numUsers; i++) {
		UserEntry& curr = m_users[i];
		curr.id.resize(Config::CLIENT_ID_SZ);
		curr.name.resize(Config::NAME_MAX_SZ);

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::CLIENT_ID_SZ, curr.id.begin());
		offset += Config::CLIENT_ID_SZ;

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::NAME_MAX_SZ, curr.name.begin());
		offset += Config::NAME_MAX_SZ;

		// Find the string terminator and resize the string to the appropriate length
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
	// Copy the client ID and public key from the byte array
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
	// Copy the target ID and message ID from the byte array
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
	// Parse the byte array to extract the message entries
	size_t offset{ 0 };
	while (offset < bytes.size()) {
		MessageEntry msg;
		msg.senderId.resize(Config::CLIENT_ID_SZ);

		// Copy the sender ID from the byte array
		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::CLIENT_ID_SZ, msg.senderId.begin());
		offset += Config::CLIENT_ID_SZ;

		// Deserialize the message id, type and content size
		msg.msgId = Utils::deserializeTrivialType<uint32_t>(bytes, offset);
		msg.msgType = MessageTypes(Utils::deserializeTrivialType<uint8_t>(bytes, offset));
		msg.contentSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);

		// Resize the content string and copy the content from the byte array
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
	: m_state{ state }
{
}

std::string ToStringVisitor::getString()
{
	// Get the string result and clear the stream
	std::string result = m_ss.str();
	m_ss.clear();
	return result;
}

void ToStringVisitor::visit(const RegistrationResPayload& payload)
{
	// Convert the UUID to a hex string, later it'll be save to the client data file
	m_ss << boost::algorithm::hex(payload.getUUID());
}

void ToStringVisitor::visit(const UsersListResPayload& payload)
{
	// If there are no other clients that a registered
	if (payload.getUsers().empty()) {
		m_ss << "There are no other registered clients at the moment";
		return;
	}

	// Iterate over the user list and print the client ID and name
	for (const auto& user : payload.getUsers()) {
		m_ss << boost::algorithm::hex(user.id) << '\t' << user.name << '\n';
	}
}

void ToStringVisitor::visit(const PublicKeyResPayload& payload)
{
	// For debugging
	m_ss << boost::algorithm::hex(payload.getPubKeyEntry().id) << '\t' << payload.getPubKeyEntry().pubKey << '\n';
}

void ToStringVisitor::visit(const MessageSentResPayload& payload)
{
	// For debugging
	m_ss << boost::algorithm::hex(payload.getMessage().targetId) << '\t' << payload.getMessage().msgId;
}

void ToStringVisitor::visit(const PollMessageResPayload& payload)
{
	// Iterate over the messages and print the sender name and message content
	auto messages = payload.getMessages();
	for (size_t i = 0; i < messages.size(); i++) {
		m_ss << "From: " << m_state.getNameByUUID(messages[i].senderId) << '\n';
		m_ss << "Content:\n";

		switch (messages[i].msgType) {
		case MessageTypes::SEND_TXT: {
			// Get the sender name and sym key
			auto username = m_state.getNameByUUID(messages[i].senderId);
			auto symKey = m_state.getSymKey(username);

			// If there is no sym key, print an error message
			if (!symKey) {
				m_ss << "can't decrypt message";
				break;
			}

			// Get the content and decrypt it using the sym key
			auto msg = messages[i].content;
			auto msgSz = msg.size();
			auto keySz = symKey.value().size();
			AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.value().c_str()), static_cast<unsigned int>(keySz));

			m_ss << aes.decrypt(msg.c_str(), static_cast<unsigned int>(msgSz));
			break;
		}
		case MessageTypes::GET_SYM_KEY:
			m_ss << "Request for symmetric key";
			break;
		case MessageTypes::SEND_SYM_KEY:
			m_ss << "Symmetric key received";
			break;
		case MessageTypes::SEND_FILE: {
			// Get the sender name and sym key
			auto username = m_state.getNameByUUID(messages[i].senderId);
			auto symKey = m_state.getSymKey(username);

			// If there is no sym key, print an error message
			if (!symKey) {
				m_ss << "can't decrypt message";
				break;
			}

			// Create a unique filename and save the file to the temp directory
			auto path = Utils::getUniquePath(messages[i].msgId);
			std::ofstream file{ path, std::ios::binary };

			// If the file can't be opened, throw a runtime error
			if (!file.is_open()) {
				throw std::runtime_error("Error: Could not open '" + path.string() + "'");
			}

			// Decrypt the file content and save it to the file
			const auto& msg = messages[i].content;
			auto msgSz = msg.size();
			auto keySz = symKey.value().size();
			AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.value().c_str()), static_cast<unsigned int>(keySz));

			file << aes.decrypt(msg.c_str(), static_cast<unsigned int>(msgSz));
			file.close();

			// Print the file path
			m_ss << "File saved to: " << path;
			break;
		}
		default:
			break;
		}

		m_ss << "\n-----<EOM>-----\n\n";
	}
}

void ToStringVisitor::visit(const ErrorPayload& payload)
{
	// Print a generic error message
	m_ss << std::string("Server responded with a generic error");
}

ClientStateVisitor::ClientStateVisitor(ClientState& state)
	: m_state{ state }
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
	// Get the public key entry and set the public key for the client
	auto entry = payload.getPubKeyEntry();
	auto name = m_state.getNameByUUID(entry.id);
	m_state.setPubKey(name, entry.pubKey);
}

void ClientStateVisitor::visit(const PollMessageResPayload& payload)
{
	// Iterate over the messages, if the message is a symmetric key, decrypt it and save it in the client state so messages/files could also be decrypted
	auto messages = payload.getMessages();
	for (size_t i = 0; i < messages.size(); i++) {
		switch (messages[i].msgType) {
		case MessageTypes::SEND_SYM_KEY: {
			auto privKey = m_state.getPrivKey();
			auto rsaprive = RSAPrivateWrapper(privKey);
			auto username = m_state.getNameByUUID(messages[i].senderId);

			m_state.setSymKey(username, rsaprive.decrypt(messages[i].content));
			break;
		}
		default:
			break;
		}
	}
}

void ClientStateVisitor::visit(const RegistrationResPayload& payload)
{
}

void ClientStateVisitor::visit(const MessageSentResPayload& payload)
{
}

void ClientStateVisitor::visit(const ErrorPayload& payload)
{
}
