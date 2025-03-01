#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/hex.hpp>

#include "Request.h"
#include "Response.h"
#include "ResPayload.h"
#include "ReqPayload.h"
#include "Config.h"
#include "Utils.h"
#include "Connection.h"

using boost::asio::ip::tcp;

int main()
{
	try {
		boost::asio::io_context ctx;
		Connection conn{ ctx, "localhost", "1234" };

		std::string name = "Michael Jackson";
		std::string pubKey = "secret_key = hello123";

		conn.addRequestHandler(RequestCodes::REGISTER, [&name, &pubKey](Connection* conn, RequestCodes code) {
			Request req{ std::string(Config::CLIENT_ID_SZ, 0),
				code,
				std::make_unique<RegisterReqPayload>(name, pubKey) };

			conn->send(req);
			return conn->recvResponse();
		});

		conn.addRequestHandler(RequestCodes::USRS_LIST, [](Connection* conn, RequestCodes code) {
			Request req{ std::string(Config::CLIENT_ID_SZ, 0),
				code,
				std::make_unique<UsersListReqPayload>()};

			conn->send(req);
			return conn->recvResponse();
		});

		std::string id{"729E2797ABE345BA87E3472C270ADE36"};
		std::string unhex{};
		boost::algorithm::unhex(id, std::back_inserter(unhex));
		conn.addRequestHandler(RequestCodes::GET_PUB_KEY, [&unhex](Connection* conn, RequestCodes code) {
			Request req{ std::string(Config::CLIENT_ID_SZ, 0),
			code,
			std::make_unique<GetPublicKeyReqPayload>(unhex) };

			conn->send(req);
			return conn->recvResponse();
		});

		auto res = conn.dispatch(RequestCodes::USRS_LIST);

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
		case ResponseCodes::PUB_KEY: {
			auto pubKey = dynamic_cast<const PublicKeyResPayload*>(&res.getPayload());
			if (pubKey) {
				std::cout << boost::algorithm::hex(pubKey->getPubKeyEntry().id) << '\t' << pubKey->getPubKeyEntry().pubKey << '\n';
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