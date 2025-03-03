#include "Client.h"

#include <iostream>


int main()
{
	try {
		boost::asio::io_context ctx;
		Client client{ ctx, "localhost", "1234" };

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