#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/asio.hpp>

#include "Response.h"

enum class RequestCodes : uint16_t;

class Connection
{
public:
	using io_ctx_t = boost::asio::io_context;
	using resolver_t = boost::asio::ip::tcp::resolver;
	using socket_t = boost::asio::ip::tcp::socket;
	using header_t = Response::Header;
	using bytes_t = std::vector<uint8_t>;
	using response_t = Response;
	using handler_map_t = std::unordered_map<uint16_t, std::function<response_t(Connection*, RequestCodes)>>;

	Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port);

	void addRequestHandler(RequestCodes code, std::function<response_t(Connection*, RequestCodes)> handler);
	response_t dispatch(RequestCodes code);

	void send(const bytes_t& bytes);
	header_t readHeader();
	bytes_t readPayload(header_t header);

private:
	size_t recv(bytes_t& outBytes, size_t recvSz);

private:
	io_ctx_t& m_ctx;
	resolver_t m_resolver;
	socket_t m_socket;

	handler_map_t m_handlerMap;
};

