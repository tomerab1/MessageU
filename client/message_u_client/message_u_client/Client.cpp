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
	m_cli->addHandler(CLIMenuOpts::REGISTER, "Register", std::bind(&Client::onCliRegister, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::REQ_CLIENT_LIST, "Request for clients list", std::bind(&Client::onCliReqClientList, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::REQ_PUB_KEY, "Request for public key", std::bind(&Client::onCliReqPubKey, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::REQ_PENDING_MSGS, "Request for waiting messages", std::bind(&Client::onCliReqPendingMsgs, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::SEND_TEXT, "Send a text message", std::bind(&Client::onCliSetTextMsg, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::REQ_SYM_KEY, "Send a request for symmetric key", std::bind(&Client::onCliReqSymKey, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::SEND_SYM_KEY, "Send your symmetric key", std::bind(&Client::onCliSendSymKey, this, std::ref(*m_cli)));
	m_cli->addHandler(CLIMenuOpts::EXIT, "Exit client", [](CLI& cli) {});
}

void Client::onCliRegister(CLI& cli)
{
	auto username = cli.getStr("Enter a username: ");
	std::string pubKey = "secret_key = hello123";

	m_conn->addHandler(RequestCodes::REGISTER, [&username, &pubKey](Connection& conn, RequestCodes code) {
		Request req{ std::string(Config::CLIENT_ID_SZ, 0),
			code,
			std::make_unique<RegisterReqPayload>(username, pubKey) };

		conn.send(req);
		return conn.recvResponse();
	});

	auto res = m_conn->dispatch(RequestCodes::REGISTER);
	std::cout << res.getPayload().toString() << '\n';
}

void Client::onCliReqClientList(CLI& cli)
{
}

void Client::onCliReqPubKey(CLI& cli)
{
}

void Client::onCliReqPendingMsgs(CLI& cli)
{
}

void Client::onCliSetTextMsg(CLI& cli)
{
}

void Client::onCliReqSymKey(CLI& cli)
{
}

void Client::onCliSendSymKey(CLI& cli)
{
}


Client::~Client() = default;