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
#include <boost/algorithm/hex.hpp>

Client::Client(context_t& ctx, const std::string& addr, const std::string& port)
	: m_cli{ std::make_unique<CLI>("MessageU client at your service", "?") },
	m_conn{ std::make_unique<Connection>(ctx, addr, port)},
	m_state{Config::ME_DOT_INFO_PATH}
{
	if (!m_state.isInitialized()) {
		getCLI().addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this));
		getCLI().addHandler(CLIMenuOpts::EXIT, "Exit client", []() {});
	}
	else {
		setupCliHandlers();
	}
}

void Client::run()
{
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
	getCLI().addHandler(CLIMenuOpts::EXIT, "Exit client", []() {});
}

void Client::onCliRegister()
{
	auto username = getCLI().input("Enter a username: ");
	RSAPrivateWrapper rsapriv;
	std::string pubKey = rsapriv.getPublicKey();

	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::REGISTER,
		std::make_unique<RegisterReqPayload>(username, pubKey) };

	getConn().send(req);
	auto res = getConn().recvResponse();

	auto payloadVisitor = std::make_unique<ToStringVisitor>(getState());
	res.getPayload().accept(*payloadVisitor);

	if (res.getHeader().code == ResponseCodes::REG_OK) {
		auto uuid = payloadVisitor->getString();

		getState().setUsername(username);
		getState().setPubKey(pubKey);
		getState().setPrivKey(rsapriv.getPrivateKey());
		getState().setUUID(uuid);
		getState().saveToFile(Config::ME_DOT_INFO_PATH);

		setupCliHandlers();
	} else {
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

	auto stringVisitor = std::make_unique<ToStringVisitor>(getState());
	auto stateVisitor = std::make_unique<ClientStateVisitor>(getState());

	res.getPayload().accept(*stringVisitor);
	res.getPayload().accept(*stateVisitor);

	std::cout << stringVisitor->getString() << '\n';
}

void Client::onCliReqPubKey()
{
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);

	Request req{ getState().getUUIDUnhexed(),
		RequestCodes::GET_PUB_KEY,
		std::make_unique<GetPublicKeyReqPayload>(targetUUID) };

	getConn().send(req);
	auto res = getConn().recvResponse();

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

	auto stringVisitor = std::make_unique<ToStringVisitor>(getState());
	auto stateVisitor = std::make_unique<ClientStateVisitor>(getState());

	res.getPayload().accept(*stateVisitor);
	res.getPayload().accept(*stringVisitor);

	std::cout << stringVisitor->getString() << '\n';
}

void Client::onCliSendTextMsg()
{
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);
	auto msgContent = getCLI().input("Enter your message: ");
	auto symKey = getState().getSymKey(targetUsername);

	AESWrapper aes(reinterpret_cast<const uint8_t*>(symKey.c_str()), symKey.size());
	auto encryptedMsg = aes.encrypt(msgContent.c_str(), msgContent.size());

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::SEND_TXT, encryptedMsg.size(), encryptedMsg)};

	getConn().send(req);
	getConn().recvResponse();
}

void Client::onCliReqSymKey()
{
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::GET_SYM_KEY, 0, "")};

	getConn().send(req);
	getConn().recvResponse();
}

void Client::onCliSendSymKey()
{
	auto targetUsername = getCLI().input("Enter a username: ");
	auto targetUUID = getState().getUUID(targetUsername);
	auto targetPubKey = getState().getPubKey(targetUsername);

	if (getState().getSymKey(targetUsername).empty()) {
		unsigned char key[AESWrapper::DEFAULT_KEYLENGTH];
		AESWrapper aes(AESWrapper::GenerateKey(key, AESWrapper::DEFAULT_KEYLENGTH), AESWrapper::DEFAULT_KEYLENGTH);
		std::string symKey;
		symKey.resize(AESWrapper::DEFAULT_KEYLENGTH);
		std::copy(std::begin(key), std::end(key), symKey.begin());

		getState().setSymKey(targetUsername, symKey);
	}

	auto symKey = getState().getSymKey(targetUsername);
	auto rsaPub = RSAPublicWrapper(targetPubKey);
	auto encryptedSymKey = rsaPub.encrypt(symKey);

	Request req{ getState().getUUIDUnhexed(),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>(targetUUID, MessageTypes::SEND_SYM_KEY, encryptedSymKey.size(), encryptedSymKey) };

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

	std::getline(ss, line);
	setUsername(line);

	std::getline(ss, line);
	setUUID(line);

	std::string priKey{ std::istreambuf_iterator<char>(ss), std::istreambuf_iterator<char>() };
	setPrivKey(Base64Wrapper::decode(priKey));
}

void ClientState::saveToFile(const std::filesystem::path& path)
{
	m_isInitialized = true;
	std::ofstream out{ path };

	if (!out.is_open()) {
		throw std::runtime_error("Error: Failed to load client state from '" + path.filename().string() + "'");
	}

	out << getUsername() << std::endl;
	out << getUUID() << std::endl;
	out << Base64Wrapper::encode(getPrivKey()) << std::endl;
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
	auto iter = m_uuidToName.find(uuid);
	if (iter == m_uuidToName.end()) {
		throw std::runtime_error("Error: Can't find user with uuid='" + uuid + "'");
	}

	return iter->second;
}

void ClientState::addClient(const std::string& name, const std::string& uuid)
{
	if (m_nameToClient.find(name) != m_nameToClient.end()) {
		return;
	}

	ClientEntry other;
	other.uuid = uuid;

	m_nameToClient.insert({ name, other });
	m_uuidToName.insert({ other.uuid, name });
}

void ClientState::setUsername(const std::string& username)
{
	m_store[ClientStateKeys::USERNAME] = username;
}

void ClientState::setUUID(const std::string& uuid)
{
	m_store[ClientStateKeys::UUID] = uuid;
}

void ClientState::setPubKey(const std::string& pubKey)
{
	m_store[ClientStateKeys::PUB_KEY] = pubKey;
}

void ClientState::setPubKey(const std::string& username, const std::string& pubKey)
{
	getClient(username).pubKey = pubKey;
}

void ClientState::setPrivKey(const std::string& privKey)
{
	m_store[ClientStateKeys::PRIV_KEY] = privKey;
}

void ClientState::setSymKey(const std::string& username, const std::string& symKey)
{
	getClient(username).symKey = symKey;
}

const std::string& ClientState::getUsername()
{
	return m_store[ClientStateKeys::USERNAME];
}

std::string ClientState::getUUIDUnhexed()
{
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
		// Return an empty uuid (for first time before registration).
		return Config::EMPTY_UUID;
	}

	return m_store[ClientStateKeys::UUID];
}

const std::string& ClientState::getUUID(const std::string& username)
{
	return getClient(username).uuid;
}

const std::string& ClientState::getPubKey()
{
	return m_store[ClientStateKeys::PUB_KEY];
}

const std::string& ClientState::getPubKey(const std::string& username)
{
	return getClient(username).pubKey;
}

const std::string& ClientState::getPrivKey()
{
	return m_store[ClientStateKeys::PRIV_KEY];
}

const std::string& ClientState::getSymKey(const std::string& username)
{
	return getClient(username).symKey;
}

ClientState::ClientEntry& ClientState::getClient(const std::string& username)
{
	auto iter = m_nameToClient.find(username);
	if (iter == m_nameToClient.end()) {
		throw std::runtime_error("Error: Can't find username: '" + username + "'");
	}

	return iter->second;
}
