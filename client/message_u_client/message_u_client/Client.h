#pragma once

#include <vector>
#include <memory>
#include <boost/asio.hpp>

class CLI;
class Connection;

class Client
{
public:
	using context_t = boost::asio::io_context;
	using cli_t = std::unique_ptr<CLI>;
	using connection_t = std::unique_ptr<Connection>;

	Client(context_t& ctx, const std::string& addr, const std::string& port);

	void run();

	~Client();

private:
	CLI& getCLI();
	Connection& getConn();

	void setupCliHandlers();

	void onCliRegister();
	void onCliReqClientList();
	void onCliReqPubKey();
	void onCliReqPendingMsgs();
	void onCliSetTextMsg();
	void onCliReqSymKey();
	void onCliSendSymKey();

private:
	cli_t m_cli;
	connection_t m_conn;
};

