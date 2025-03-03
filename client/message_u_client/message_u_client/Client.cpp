#include "Client.h"
#include "CLI.h"
#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "ReqPayload.h"

#include <iostream>
#include <sstream>
#include <fstream>

Client::Client(context_t& ctx, const std::string& addr, const std::string& port)
	: m_cli{ std::make_unique<CLI>("MessageU client at your service", "?") },
	m_conn{ std::make_unique<Connection>(ctx, addr, port)},
	m_state{Config::ME_DOT_INFO_PATH}
{
	if (!m_state.isInitialized()) {
		m_cli->addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this));
		m_cli->addHandler(CLIMenuOpts::EXIT, "Exit client", []() {});
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
	m_cli->addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this));
	m_cli->addHandler(CLIMenuOpts::REQ_CLIENT_LIST, "Request for clients list", std::bind(&Client::onCliReqClientList, this));
	m_cli->addHandler(CLIMenuOpts::REQ_PUB_KEY, "Request for public key", std::bind(&Client::onCliReqPubKey, this));
	m_cli->addHandler(CLIMenuOpts::REQ_PENDING_MSGS, "Request for waiting messages", std::bind(&Client::onCliReqPendingMsgs, this));
	m_cli->addHandler(CLIMenuOpts::SEND_TEXT, "Send a text message", std::bind(&Client::onCliSendTextMsg, this));
	m_cli->addHandler(CLIMenuOpts::REQ_SYM_KEY, "Send a request for symmetric key", std::bind(&Client::onCliReqSymKey, this));
	m_cli->addHandler(CLIMenuOpts::SEND_SYM_KEY, "Send your symmetric key", std::bind(&Client::onCliSendSymKey, this));
	m_cli->addHandler(CLIMenuOpts::EXIT, "Exit client", []() {});
}

void Client::onCliRegister()
{
	auto username = getCLI().input("Enter a username: ");
	std::string pubKey = "secret_key = hello123";

	Request req{ m_state.getUUID(),
		RequestCodes::REGISTER,
		std::make_unique<RegisterReqPayload>(username, pubKey) };

	getConn().send(req);
	auto res = getConn().recvResponse();

	if (res.getHeader().code == ResponseCodes::REG_OK) {
		auto uuid = res.getPayload().toString();

		m_state.setUsername(username);
		m_state.setPubKey(pubKey);
		m_state.setUUID(uuid);
		m_state.saveToFile(Config::ME_DOT_INFO_PATH, username, uuid, pubKey);

		setupCliHandlers();
	} else {
		std::cout << res.getPayload().toString() << "\n\n";
	}
}

void Client::onCliReqClientList()
{
	Request req{ m_state.getUUID(),
		RequestCodes::USRS_LIST,
		std::make_unique<UsersListReqPayload>() };

	getConn().send(req);
	auto res = getConn().recvResponse();

	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqPubKey()
{
	//Request req{ std::string(Config::CLIENT_ID_SZ, 0), RequestCodes::GET_PUB_KEY, std::make_unique<GetPublicKeyReqPayload>() };

	//getConn().send(req);
	//auto res = getConn().recvResponse();

	//std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqPendingMsgs()
{
	Request req{ m_state.getUUID(),
	RequestCodes::POLL_MSGS,
	std::make_unique<PollMessagesReqPayload>() };

	getConn().send(req);
	auto res = getConn().recvResponse();

	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliSendTextMsg()
{
	auto msgContent = getCLI().input("Enter your message: ");

	Request req{ std::string(Config::CLIENT_ID_SZ, 0),
			RequestCodes::SEND_MSG,
			std::make_unique<SendMessageReqPayload>("", MessageTypes::SEND_TXT, msgContent.size(), msgContent)};

	getConn().send(req);
	auto res = getConn().recvResponse();

	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqSymKey()
{
	//Request req{ std::string(Config::CLIENT_ID_SZ, 0),
	//		RequestCodes::SEND_MSG,
	//		std::make_unique<SendMessageReqPayload>("", MessageTypes::GET_SYM_KEY) };

	//getConn().send(req);
	//auto res = getConn().recvResponse();

	//std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliSendSymKey()
{
	//Request req{ std::string(Config::CLIENT_ID_SZ, 0),
	//		RequestCodes::SEND_MSG,
	//		std::make_unique<SendMessageReqPayload>("", MessageTypes::SEND_SYM_KEY) };

	//getConn().send(req);
	//auto res = getConn().recvResponse();

	//std::cout << res.getPayload().toString() << '\n';
}

CLI& Client::getCLI()
{
	return *m_cli;
}

Connection& Client::getConn()
{
	return *m_conn;
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

	std::getline(ss, line);
	setPrivKey(line);
}

void ClientState::saveToFile(const std::filesystem::path& path, const std::string& username, const std::string& uuid, const std::string& privKey)
{
	m_isInitialized = true;
	std::ofstream out{ path };

	if (!out.is_open()) {
		throw std::runtime_error("Error: Failed to load client state from '" + path.filename().string() + "'");
	}

	out << getUsername() << std::endl;
	out << getUUID() << std::endl;
	out << getPrivKey() << std::endl;
}

bool ClientState::isInitialized()
{
	return m_isInitialized;
}

void ClientState::addOtherClient(const std::string& name, const std::string& uuid)
{
	if (m_nameToClient.find(name) != m_nameToClient.end()) {
		return;
	}

	OtherClientEntry other;
	other.uuid = uuid;

	m_nameToClient.insert({ name, OtherClientEntry {} });
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
	if (m_nameToClient.find(username) == m_nameToClient.end()) {
		throw std::runtime_error("Error: Can't find username: '" + username + "'");
	}

	m_nameToClient[username].pubKey = pubKey;
}

void ClientState::setPrivKey(const std::string& privKey)
{
	m_store[ClientStateKeys::PRIV_KEY] = privKey;
}

void ClientState::setSymKey(const std::string& symKey)
{
	m_store[ClientStateKeys::SYM_KEY] = symKey;
}

const std::string& ClientState::getUsername()
{
	return m_store[ClientStateKeys::USERNAME];
}

const std::string& ClientState::getUUID()
{
	if (m_store.find(ClientStateKeys::UUID) == m_store.end()) {
		// Return an empty uuid (for first time before registration).
		std::string emptyUUID;
		emptyUUID.resize(0);
		return emptyUUID;
	}

	return m_store[ClientStateKeys::UUID];
}

const std::string& ClientState::getUUID(const std::string& username)
{
	if (m_nameToClient.find(username) == m_nameToClient.end()) {
		throw std::runtime_error("Error: Can't find username: '" + username + "'");
	}

	return m_nameToClient[username].uuid;
}

const std::string& ClientState::getPubKey()
{
	return m_store[ClientStateKeys::PUB_KEY];
}

const std::string& ClientState::getPubKey(const std::string& username)
{
	if (m_nameToClient.find(username) == m_nameToClient.end()) {
		throw std::runtime_error("Error: Can't find username: '" + username + "'");
	}

	return m_nameToClient[username].pubKey;
}

const std::string& ClientState::getPrivKey()
{
	return m_store[ClientStateKeys::PRIV_KEY];
}

const std::string& ClientState::getSymKey()
{
	return m_store[ClientStateKeys::SYM_KEY];
}
