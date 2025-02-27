#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main()
{
	boost::asio::io_context ctx;
	tcp::socket sock{ ctx };
	tcp::resolver resolver{ ctx };

	boost::asio::connect(sock, resolver.resolve("localhost", "1234"));
	std::cout << "Connected\n";

	return 0;
}
