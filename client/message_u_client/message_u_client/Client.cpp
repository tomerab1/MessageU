#include "Client.h"
#include "CLI.h"
#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "ReqPayload.h"
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <boost/algorithm/hex.hpp>

Client::Client(context_t& ctx, const std::string& addr, const std::string& port)
	: m_cli{ std::make_unique<CLI>("MessageU client at your service", "?") },
	m_conn{ std::make_unique<Connection>(ctx, addr, port) },
	m_state{ Config::ME_DOT_INFO_PATH }
{
	// Setting up the cli handlers.
	setupCliHandlers();
}

void Client::run()
{
	// Getting the cli and running it, to enable client interaction.
	getCLI().run();
}

void Client::setupCliHandlers()
{
	// Binding the cli events to their handlers.
	getCLI().addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this));
	getCLI().addHandler(CLIMenuOpts::REQ_CLIENT_LIST, "Request for clients list", std::bind(&Client::onCliReqClientList, this));
	getCLI().addHandler(CLIMenuOpts::REQ_PUB_KEY, "Request for public key", std::bind(&Client::onCliReqPubKey, this));
	getCLI().addHandler(CLIMenuOpts::REQ_PENDING_MSGS, "Request for waiting messages", std::bind(&Client::onCliReqPendingMsgs, this));
	getCLI().addHandler(CLIMenuOpts::SEND_TEXT, "Send a text message", std::bind(&Client::onCliSendTextMsg, this));
	getCLI().addHandler(CLIMenuOpts::REQ_SYM_KEY, "Send a request for symmetric key", std::bind(&Client::onCliReqSymKey, this));
	getCLI().addHandler(CLIMenuOpts::SEND_SYM_KEY, "Send your symmetric key", std::bind(&Client::onCliSendSymKey, this));
	getCLI().addHandler(CLIMenuOpts::SEND_FILE, "Send a file", std::bind(&Client::onCliSendFile, this));
	getCLI().addHandler(CLIMenuOpts::EXIT, "Exit client", []() {});
}

void Client::onCliRegister()
{
	// Getting the username from the user.
	auto username = getCLI().input("Enter a username: ");

	// Checking if the username is valid.
	if (username.length() >= Config::NAME_MAX_SZ) {
		throw std::logic_error("Error: Name length is '" + std::to_string(username.length()) + "' but the max is '" + std::to_string(Config::NAME_MAX_SZ) + "'");
	}

	// Creating a new RSA key pair.
	RSAPrivateWrapper rsapriv;
	std::string pubKey = rsapriv.getPublicKey();

	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::REGISTER,
		std::make_unique<RegisterReqPayload>(username, pubKey) };

	getConn().send(req);
	auto res = getConn().recvResponse();

	// Getting the payload of the response, if the response is successful, save the user info to a file.
	// Else, print the error message.
	auto payloadVisitor = std::make_unique<ToStringVisitor>(getState());
	res.getPayload().accept(*payloadVisitor);

	if (res.getHeader().code == ResponseCodes::REG_OK) {
		auto uuid = payloadVisitor->getString();

		getState().setUsername(username);
		getState().setPubKey(pubKey);
		getState().setPrivKey(rsapriv.getPrivateKey());
		getState().setUUID(uuid);
		getState().saveToFile(Config::ME_DOT_INFO_PATH);
	}
	else {
		std::cout << payloadVisitor->getString() << "\n\n";
	}
}

void Client::onCliReqClientList()
{
	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::USRS_LIST,
		std::make_unique<UsersListReqPayload>() };

	getConn().send(req);
	auto res = getConn().recvResponse();

	// Visiting the payload using the ToStringVisitor to print the clients list and the ClientStateVisitor to update the client state.
	auto stringVisitor = std::make_unique<ToStringVisitor>(getState());
	auto stateVisitor = std::make_unique<ClientStateVisitor>(getState());

	res.getPayload().accept(*stringVisitor);
	res.getPayload().accept(*stateVisitor);

	std::cout << stringVisitor->getString() << '\n';
}

void Client::onCliReqPubKey()
{
	// Getting the target username from the user and extracting the target UUID from the client state.
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);

	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::GET_PUB_KEY,
		std::make_unique<GetPublicKeyReqPayload>(targetUUID) };

	getConn().send(req);
	auto res = getConn().recvResponse();

	// Update the state with the public key of the target user.
	auto stateVisitor = std::make_unique<ClientStateVisitor>(getState());
	res.getPayload().accept(*stateVisitor);
}

void Client::onCliReqPendingMsgs()
{
	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::POLL_MSGS,
		std::make_unique<PollMessagesReqPayload>() };

	getConn().send(req);
	auto res = getConn().recvResponse();

	// Visiting the payload using the ToStringVisitor to print the pending messages and the ClientStateVisitor to update the client state.
	auto stringVisitor = std::make_unique<ToStringVisitor>(getState());
	auto stateVisitor = std::make_unique<ClientStateVisitor>(getState());

	res.getPayload().accept(*stateVisitor);
	res.getPayload().accept(*stringVisitor);

	std::cout << stringVisitor->getString() << '\n';
}

void Client::onCliSendTextMsg()
{
	// Getting the target username from the user and extracting the target UUID from the client state.
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);
	
	// Getting the message content from the user and the symmetric key from the client state.
	auto msgContent = getCLI().input("Enter your message: ");
	auto symKey = getState().getSymKey(targetUsername);

	// If the symmetric key doesn't exist, throw an error.
	if (!symKey) {
		throw std::logic_error("Error: Can't get the symmetric key of '" + targetUsername + "' it doesn't exist yet");
	}

	// Encrypt the message content using the symmetric key and send it to the server.
	auto symKeySz = symKey.value().size();
	auto msgSz = msgContent.size();
	AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.value().c_str()), static_cast<unsigned int>(symKeySz));
	auto encryptedMsg = aes.encrypt(msgContent.c_str(), static_cast<unsigned int>(msgSz));

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::SEND_TXT, encryptedMsg.size(), encryptedMsg) };

	getConn().send(req);
	getConn().recvResponse();
}

void Client::onCliReqSymKey()
{
	// Getting the target username from the user and extracting the target UUID from the client state.
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);

	// Send a request to the server to get the symmetric key of the target user.
	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::GET_SYM_KEY, 0, "") };

	getConn().send(req);
	getConn().recvResponse();
}

void Client::onCliSendSymKey()
{
	// Getting the target username from the user and extracting the target UUID and public key from the client state.
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);
	auto targetPubKey = getState().getPubKey(targetUsername);

	// If the public key doesn't exist, throw an error.
	if (!targetPubKey) {
		throw std::logic_error("Error: Can't get the public key of '" + targetUsername + "' it doesn't exist yet");
	}

	// If the symmetric key doesn't exist, generate a new one and save it to the client state.
	if (!getState().getSymKey(targetUsername)) {
		unsigned char key[AESWrapper::DEFAULT_KEYLENGTH];
		AESWrapper aes(AESWrapper::GenerateKey(key, AESWrapper::DEFAULT_KEYLENGTH), AESWrapper::DEFAULT_KEYLENGTH);
		std::string symKey;
		symKey.resize(AESWrapper::DEFAULT_KEYLENGTH);
		std::copy(std::begin(key), std::end(key), symKey.begin());

		getState().setSymKey(targetUsername, symKey);
	}

	// Encrypt the symmetric key using the target user's public key and send it to the server.
	auto symKey = getState().getSymKey(targetUsername).value();
	auto rsaPub = RSAPublicWrapper(targetPubKey.value());
	auto encryptedSymKey = rsaPub.encrypt(symKey);

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::SEND_SYM_KEY, encryptedSymKey.size(), encryptedSymKey) };

	getConn().send(req);
	getConn().recvResponse();
}

void Client::onCliSendFile()
{
	// Getting the target username from the user and extracting the target UUID and symmetric key from the client state.
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);

	// Getting the file path from the user and the symmetric key from the client state.
	auto path = getCLI().input("Enter file path: ");
	auto symKey = getState().getSymKey(targetUsername);

	// If the symmetric key doesn't exist, throw an error.
	if (!symKey) {
		throw std::logic_error("Error: Can't get the symmetric key of '" + targetUsername + "' it doesn't exist yet");
	}

	// Read the file content and encrypt it using the symmetric key and send it to the server.
	std::ifstream file{ path, std::ios::binary | std::ios::beg };
	std::stringstream ss;

	ss << file.rdbuf();
	auto msgContent = ss.str();
	auto symKeySz = symKey.value().size();
	auto msgSz = msgContent.size();
	AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.value().c_str()), static_cast<unsigned int>(symKeySz));
	auto encryptedMsg = aes.encrypt(msgContent.c_str(), static_cast<unsigned int>(msgSz));

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::SEND_FILE, encryptedMsg.size(), encryptedMsg) };

	getConn().send(req);
	getConn().recvResponse();
}

CLI& Client::getCLI()
{
	return *m_cli;
}

Connection& Client::getConn()
{
	return *m_conn;
}

ClientState& Client::getState()
{
	return m_state;
}

Client::~Client() = default;

ClientState::ClientState(const std::filesystem::path& path)
{
	// Check if the file exists, if it does, load the client state from it.
	if (std::filesystem::exists(path)) {
		loadFromFile(path);
	}
}

void ClientState::loadFromFile(const std::filesystem::path& path)
{
	m_isInitialized = true;
	std::ifstream in{ path };

	if (!in.is_open()) {
		throw std::runtime_error("Error: Failed to load client state from '" + path.filename().string() + "'");
	}

	std::stringstream ss;
	ss << in.rdbuf();

	std::string line;

	// Read the uername
	std::getline(ss, line);

	if (line.size() > Config::NAME_MAX_SZ || line.empty()) {
		throw std::runtime_error("Error: Could not load 'me.info' name is of invalid length");
	}

	setUsername(line);

	// Read the hexed uuid
	std::getline(ss, line);

	// * 2 because each byte is encoded using 2 hex characters
	if (line.size() != Config::CLIENT_ID_SZ * 2 || line.empty()) {
		throw std::runtime_error("Error: Could not load 'me.info' UUID is of invalid length");
	}

	setUUID(line);

	// Read the private key
	std::string priKey{ std::istreambuf_iterator<char>(ss), std::istreambuf_iterator<char>() };
	auto decodedKey = Base64Wrapper::decode(priKey);

	if (decodedKey.empty()) {
		throw std::runtime_error("Error: Could not load 'me.info' private key is missing");
	}

	setPrivKey(decodedKey);
}

void ClientState::saveToFile(const std::filesystem::path& path)
{
	m_isInitialized = true;
	std::ofstream out{ path };

	if (!out.is_open()) {
		throw std::runtime_error("Error: Failed to load client state from '" + path.filename().string() + "'");
	}

	// Write the username, hexed uuid and private key to the file.
	out << getUsername() << std::endl;
	out << getUUID() << std::endl;
	out << Base64Wrapper::encode(getPrivKey());
}

bool ClientState::isInitialized()
{
	return m_isInitialized;
}

bool ClientState::hasSymKey(const std::string& username)
{
	return true;
}

std::string ClientState::getNameByUUID(const std::string& uuid)
{
	// Find the username by the uuid.
	auto iter = m_uuidToName.find(uuid);
	// If the uuid doesn't exist, throw an error.
	if (iter == m_uuidToName.end()) {
		throw std::runtime_error("Error: Can't find user with uuid='" + uuid + "'");
	}

	return iter->second;
}

void ClientState::addClient(const std::string& name, const std::string& uuid)
{
	// If the client already exists, return.
	if (m_nameToClient.find(name) != m_nameToClient.end()) {
		return;
	}

	// Create a new client entry and insert it to the maps.
	ClientEntry other;
	other.uuid = uuid;

	m_nameToClient.insert({ name, other });
	m_uuidToName.insert({ other.uuid, name });
}

void ClientState::setUsername(const std::string& username)
{
	// Set the username of the current client
	m_store[ClientStateKeys::USERNAME] = username;
}

void ClientState::setUUID(const std::string& uuid)
{
	// Set the uuid of the current client
	m_store[ClientStateKeys::UUID] = uuid;
}

void ClientState::setPubKey(const std::string& pubKey)
{
	// Set the public key of the current client
	m_store[ClientStateKeys::PUB_KEY] = pubKey;
}

void ClientState::setPubKey(const std::string& username, const std::string& pubKey)
{
	// Set the public key for another client
	getClient(username).pubKey = pubKey;
}

void ClientState::setPrivKey(const std::string& privKey)
{
	// Set the private key of the current client
	m_store[ClientStateKeys::PRIV_KEY] = privKey;
}

void ClientState::setSymKey(const std::string& username, const std::string& symKey)
{
	// Set the symmetric key for another client
	getClient(username).symKey = symKey;
}

const std::string& ClientState::getUsername()
{
	// Get the username of the current client
	return m_store[ClientStateKeys::USERNAME];
}

std::string ClientState::getUUIDUnhexed()
{
	// Get the uuid of the current client and convert it from hex to string
	auto uuid = getUUID();
	if (uuid == Config::EMPTY_UUID) {
		return uuid;
	}

	std::string unhex;
	boost::algorithm::unhex(uuid, std::back_inserter(unhex));
	return unhex;
}

const std::string& ClientState::getUUID()
{
	if (m_store.find(ClientStateKeys::UUID) == m_store.end()) {
		// Return an empty uuid (for first time before registration as the server generates the uuid for a client).
		return Config::EMPTY_UUID;
	}

	// Get the uuid of the current client
	return m_store[ClientStateKeys::UUID];
}

const std::string& ClientState::getUUID(const std::string& username)
{
	// Get the uuid of another client
	return getClient(username).uuid;
}

const std::string& ClientState::getPubKey()
{
	// Get the public key of the current client
	return m_store[ClientStateKeys::PUB_KEY];
}

const std::optional<std::string>& ClientState::getPubKey(const std::string& username)
{
	// Get the public key of another client
	return getClient(username).pubKey;
}

const std::string& ClientState::getPrivKey()
{
	// Get the private key of the current client
	return m_store[ClientStateKeys::PRIV_KEY];
}

const std::optional<std::string>& ClientState::getSymKey(const std::string& username)
{
	// Get the symmetric key of another client
	return getClient(username).symKey;
}

ClientState::ClientEntry& ClientState::getClient(const std::string& username)
{
	// Get the client entry of another client
	auto iter = m_nameToClient.find(username);
	// If the client doesn't exist, throw an error.
	if (iter == m_nameToClient.end()) {
		throw std::runtime_error("Error: Can't find username: '" + username + "'");
	}

	return iter->second;
}
