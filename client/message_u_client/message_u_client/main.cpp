#include "Client.h"
#include "Config.h"

#include <iostream>


int main()
{
	try {
		boost::asio::io_context ctx;
		Client client{ ctx, Config::SERVER_ADDR, Config::SERVER_PORT };

		client.run();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << '\n';
	}
	catch (...) {
		std::cout << "Unexpected error has occurred\n";
	}

	return 0;
}