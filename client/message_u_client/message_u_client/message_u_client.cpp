﻿#include <iostream>
#include <boost/asio.hpp>

#include "Request.h"
#include "RegisterPayload.h"

using boost::asio::ip::tcp;

int main()
{
	boost::asio::io_context ctx;
	tcp::socket sock{ ctx };
	tcp::resolver resolver{ ctx };

	boost::asio::connect(sock, resolver.resolve("localhost", "1234"));
	std::cout << "Connected\n";

	Request req{ std::array<uint8_t, Config::CLIENT_ID_SZ>{"123456789111111"}, 600, std::make_unique<RegisterPayload>("Tomer abokarat", "secret_key=123")};

	auto bytes = req.toBytes();

	boost::asio::write(sock, boost::asio::buffer(bytes.data(), bytes.size()));


	return 0;
}
