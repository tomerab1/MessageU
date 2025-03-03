#pragma once

#include <string>
#include <map>
#include <functional>

enum class CLIMenuOpts : uint16_t {
	REGISTER = 110,
	REQ_CLIENT_LIST = 120,
	REQ_PUB_KEY = 130,
	REQ_PENDING_MSGS = 140,
	SEND_TEXT = 150,
	REQ_SYM_KEY = 151,
	SEND_SYM_KEY = 152,
	EXIT = 0,
	INVALID = 0xffff,
};

class CLI
{
public:
	struct CLIOpt;
	using handler_t = std::function<void()>;
	using handler_map_t = std::map<uint16_t, CLIOpt>;

	CLI(const std::string& header, const std::string& footer);

	struct CLIOpt {
		CLIOpt() = default;
		CLIOpt(const std::string& msg, handler_t handler);

		std::string msg;
		handler_t handler;
	};

	void addHandler(CLIMenuOpts opt, const std::string& msg, handler_t handler);
	void clearScreen();
	std::string input(const std::string& prompt="");
	void run();

private:
	void invoke(CLIMenuOpts opt);
	void displayMenu();
	CLIMenuOpts getUserOpt();

private:
	std::string m_header;
	std::string m_footer;
	handler_map_t m_handlers;
};

