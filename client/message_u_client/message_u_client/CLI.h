#pragma once

#include <string>
#include <map>
#include <functional>

/*
	Menu options for the CLI class.
*/
enum class CLIMenuOpts : uint16_t {
	REGISTER = 110,
	REQ_CLIENT_LIST = 120,
	REQ_PUB_KEY = 130,
	REQ_PENDING_MSGS = 140,
	SEND_TEXT = 150,
	REQ_SYM_KEY = 151,
	SEND_SYM_KEY = 152,
	SEND_FILE = 153,
	EXIT = 0,
	INVALID = 0xffff,
};

/*
 * CLI class is a simple command line interface that allows the user to select from a list of options.
 * Each option is associated with a handler function that is called when the user selects the option.
 * The CLI class is used to display a menu of options to the user and to handle user input.
 */
class CLI
{
public:
	// Forward declaration and type aliases
	struct CLIOpt;
	using handler_t = std::function<void()>;
	using handler_map_t = std::map<uint16_t, CLIOpt>;

	CLI(const std::string& header, const std::string& footer);

	// Menu option struct, contains the message to display and its handler.
	struct CLIOpt {
		CLIOpt() = default;
		CLIOpt(const std::string& msg, handler_t handler);

		std::string msg;
		handler_t handler;
	};

	// Register a handler for a menu option
	void addHandler(CLIMenuOpts opt, const std::string& msg, handler_t handler);

	// Get user input and return the input
	std::string input(const std::string& prompt="");

	// Run the CLI
	void run();

private:
	// Invoke the handler associated with the given option
	void invoke(CLIMenuOpts opt);

	// Display the menu options
	void displayMenu();

	// Get the user's option
	CLIMenuOpts getUserOpt();

private:
	std::string m_header;
	std::string m_footer;
	handler_map_t m_handlers;
};

