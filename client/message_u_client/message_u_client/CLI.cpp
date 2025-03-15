#include "CLI.h"
#include "Utils.h"

#include <iostream>

CLI::CLIOpt::CLIOpt(const std::string& msg, handler_t handler)
	: msg{msg}, handler{handler}
{
}

CLI::CLI(const std::string& header, const std::string& footer)
	: m_header{header}, m_footer{footer}
{
}

void CLI::run()
{
	while (true) {
		try {
			displayMenu();

			auto currOpt = getUserOpt();
			std::cout << '\n';
			if (currOpt == CLIMenuOpts::EXIT) {
				break;
			}

			invoke(currOpt);
		}
		catch (const std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}
}

void CLI::invoke(CLIMenuOpts opt)
{
	// Convert the option to its corresponding uint16_t value
	auto optCode = Utils::EnumToUint16(opt);
	auto iter = m_handlers.find(optCode);

	// Handle invalid options
	if (iter == m_handlers.end()) {
		throw std::runtime_error("Error: '" + std::to_string(optCode) + "' is not a valid option");
	}

	// Invokde the handler
	iter->second.handler();
}

void CLI::displayMenu()
{
	// Display the header
	std::cout << m_header << "\n\n";
	
	// Iterate through the handlers and display the options
	for (const auto& [code, opt] : m_handlers) {
		std::cout << code << ") " << opt.msg << '\n';
	}

	// Display the footer
	std::cout << m_footer << '\n';
}

void CLI::addHandler(CLIMenuOpts opt, const std::string& msg, handler_t handler)
{
	// If the handler already exists, do nothing
	if (m_handlers.find(Utils::EnumToUint16(opt)) != m_handlers.end()) {
		return;
	}

	m_handlers.insert({ Utils::EnumToUint16(opt), CLIOpt {msg, handler} });
}

std::string CLI::input(const std::string& prompt)
{
	std::cout << prompt << '\n';

	std::string out;
	std::getline(std::cin, out);

	Utils::trimStr(out);
	return out;
}

CLIMenuOpts CLI::getUserOpt()
{
	// Get the user's input and return it as a CLIMenuOpts
	std::string out;
	std::getline(std::cin, out);

	try {
		Utils::trimStr(out);
		return CLIMenuOpts(Utils::strToInt(out));
	}
	catch (const std::exception&) {
		throw std::runtime_error("Error: '" + out + "' is not a valid option");
	}
}