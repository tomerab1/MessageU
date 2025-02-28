#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/hex.hpp>

#include "Request.h"
#include "Response.h"
#include "ResPayload.h"
#include "ReqPayload.h"
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

		//Request req{ std::string(Config::CLIENT_ID_SZ, 0),
		//	RequestCodes::REGISTER,
		//	std::make_unique<RegisterReqPayload>("Michael Jackson |._.|", "secret_key=123") };

		Request req{ std::string(Config::CLIENT_ID_SZ, 0),
			RequestCodes::USRS_LIST,
			std::make_unique<UsersListReqPayload>()};

		auto toWrite = req.toBytes();

		boost::asio::write(sock, boost::asio::buffer(toWrite.data(), toWrite.size()));

		std::vector<uint8_t> headerBytes(Config::RES_HEADER_SZ, 0);
		boost::asio::read(sock, boost::asio::buffer(headerBytes.data(), headerBytes.size()));
		auto header = Response::Header::fromBytes(headerBytes);

		std::vector<uint8_t> payloadBytes(header.payloadSz, 0);
		boost::asio::read(sock, boost::asio::buffer(payloadBytes.data(), payloadBytes.size()));
		auto res = Response(header, payloadBytes);

		switch (res.getPayload().getResCode()) {
		case ResponseCodes::REG_OK: {
			auto regOk = dynamic_cast<const RegistrationResPayload*>(&res.getPayload());
			if (regOk) {
				std::cout << "REG_OK: " << boost::algorithm::hex(regOk->getUUID()) << '\n';
			}
		}
		break;
		case ResponseCodes::USRS_LIST: {
			auto usrsList = dynamic_cast<const UsersListResPayload*>(&res.getPayload());
			if (usrsList) {
				for (const auto& user : usrsList->getUsers()) {
					std::cout << boost::algorithm::hex(user.id) << '\t' << user.name << '\n';
				}
			}
		}
		break;
		default:
			break;
		}

	}
	catch (const std::exception& e) {
		std::cout << e.what() << '\n';
	}


	return 0;
}