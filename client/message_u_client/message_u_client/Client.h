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
	void setupCliHandlers();

	void onCliRegister(CLI& cli);
	void onCliReqClientList(CLI& cli);
	void onCliReqPubKey(CLI& cli);
	void onCliReqPendingMsgs(CLI& cli);
	void onCliSetTextMsg(CLI& cli);
	void onCliReqSymKey(CLI& cli);
	void onCliSendSymKey(CLI& cli);

private:
	cli_t m_cli;
	connection_t m_conn;
};

