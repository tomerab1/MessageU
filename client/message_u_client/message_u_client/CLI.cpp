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
		displayMenu();

		auto currOpt = getUserOpt();
		if (currOpt == CLIMenuOpts::INVALID) {
			continue;
		}
		if (currOpt == CLIMenuOpts::EXIT) {
			break;
		}

		invoke(currOpt);
	}
}

void CLI::invoke(CLIMenuOpts opt)
{
	auto optCode = Utils::EnumToUint16(opt);
	m_handlers[optCode].handler(*this);
}

void CLI::displayMenu()
{
	std::cout << m_header << "\n\n";

	for (const auto& [code, opt] : m_handlers) {
		std::cout << code << ") " << opt.msg << '\n';
	}

	std::cout << m_footer << '\n';
}

void CLI::addHandler(CLIMenuOpts opt, const std::string& msg, std::function<void(CLI&)> handler)
{
	m_handlers.insert({ Utils::EnumToUint16(opt), CLIOpt {msg, handler} });
}

void CLI::clearScreen()
{
#if defined(WIN32) || defined(_WIN32)
	system("cls");
#else
	system("clear");
#endif
}

std::string CLI::getStr(const std::string& prompt)
{
	std::cout << prompt << '\n';

	std::string out;
	std::getline(std::cin, out);

	Utils::trimStr(out);
	return out;
}

CLIMenuOpts CLI::getUserOpt()
{
	std::string out;
	std::getline(std::cin, out);

	try {
		Utils::trimStr(out);
		return CLIMenuOpts(Utils::strToInt(out));
	}
	catch (const std::exception& e) {
		std::cout << out << " is not a valid option\n";
		return CLIMenuOpts::INVALID;
	}
}