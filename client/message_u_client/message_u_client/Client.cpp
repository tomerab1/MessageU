#include "Client.h"
#include "CLI.h"
#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "ReqPayload.h"

#include <iostream>

Client::Client(context_t& ctx, const std::string& addr, const std::string& port)
	: m_cli{ std::make_unique<CLI>("MessageU client at your service", "?") },
	m_conn{ std::make_unique<Connection>(ctx, addr, port)}
{
	setupCliHandlers();
}

void Client::run()
{
	m_cli->run();
}

void Client::setupCliHandlers()
{
	m_cli->addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this));
	m_cli->addHandler(CLIMenuOpts::REQ_CLIENT_LIST, "Request for clients list", std::bind(&Client::onCliReqClientList, this));
	m_cli->addHandler(CLIMenuOpts::REQ_PUB_KEY, "Request for public key", std::bind(&Client::onCliReqPubKey, this));
	m_cli->addHandler(CLIMenuOpts::REQ_PENDING_MSGS, "Request for waiting messages", std::bind(&Client::onCliReqPendingMsgs, this));
	m_cli->addHandler(CLIMenuOpts::SEND_TEXT, "Send a text message", std::bind(&Client::onCliSetTextMsg, this));
	m_cli->addHandler(CLIMenuOpts::REQ_SYM_KEY, "Send a request for symmetric key", std::bind(&Client::onCliReqSymKey, this));
	m_cli->addHandler(CLIMenuOpts::SEND_SYM_KEY, "Send your symmetric key", std::bind(&Client::onCliSendSymKey, this));
	m_cli->addHandler(CLIMenuOpts::EXIT, "Exit client", [](CLI& cli) {});
}

void Client::onCliRegister()
{
	auto username = getCLI().input("Enter a username: ");
	std::string pubKey = "secret_key = hello123";

	Request req{ std::string(Config::CLIENT_ID_SZ, 0),
		RequestCodes::REGISTER,
		std::make_unique<RegisterReqPayload>(username, pubKey) };

	getConn().send(req);
	auto res = getConn().recvResponse();

	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqClientList()
{
	Request req{ std::string(Config::CLIENT_ID_SZ, 0),
		RequestCodes::USRS_LIST,
		std::make_unique<UsersListReqPayload>() };

	m_conn->send(req);
	auto res = getConn().recvResponse();

	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqPubKey()
{
}

void Client::onCliReqPendingMsgs()
{
}

void Client::onCliSetTextMsg()
{
}

void Client::onCliReqSymKey()
{
}

void Client::onCliSendSymKey()
{
}


Client::~Client() = default;

CLI& Client::getCLI()
{
	return *m_cli;
}

Connection& Client::getConn()
{
	return *m_conn;
}
