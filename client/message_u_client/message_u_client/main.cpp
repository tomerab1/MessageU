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
		std::cout << res.getPayload().toString() << '\n';
	}
	catch (const std::exception& e) {
		std::cout << e.what() << '\n';
	}

	return 0;
}