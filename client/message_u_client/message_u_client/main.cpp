#include <iostream>
#include <boost/asio.hpp>

#include "Request.h"
#include "Response.h"
#include "RegisterReqPayload.h"
#include "ResPayload.h"
#include "Config.h"
#include "Utils.h"

using boost::asio::ip::tcp;

int main()
{
	try {
		boost::asio::io_context ctx;
		tcp::socket sock{ ctx };
		tcp::resolver resolver{ ctx };

		boost::asio::connect(sock, resolver.resolve("localhost", "1234"));
		std::cout << "Connected\n";

		Request req{ std::string(Config::CLIENT_ID_SZ, 0),
			RequestCodes::REGISTER,
			std::make_unique<RegisterReqPayload>("Tomer", "secret_key=123") };

		auto toWrite = req.toBytes();

		boost::asio::write(sock, boost::asio::buffer(toWrite.data(), toWrite.size()));

		std::vector<uint8_t> headerBytes(Config::RES_HEADER_SZ, 0);
		boost::asio::read(sock, boost::asio::buffer(headerBytes.data(), headerBytes.size()));
		auto header = Response::Header::fromBytes(headerBytes);

		std::vector<uint8_t> payloadBytes(header.payloadSz, 0);
		boost::asio::read(sock, boost::asio::buffer(payloadBytes.data(), payloadBytes.size()));
		auto res = Response(header, payloadBytes);

		std::cout << Utils::EnumToUint16(res.getPayload().getResCode()) << '\n';

	}
	catch (const std::exception& e) {
		std::cout << e.what() << '\n';
	}


	return 0;
}