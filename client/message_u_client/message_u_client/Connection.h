#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/asio.hpp>

#include "Response.h"

enum class RequestCodes : uint16_t;
class Request;

class Connection
{
public:
	using io_ctx_t = boost::asio::io_context;
	using resolver_t = boost::asio::ip::tcp::resolver;
	using socket_t = boost::asio::ip::tcp::socket;
	using header_t = Response::Header;
	using bytes_t = std::vector<uint8_t>;
	using handler_map_t = std::unordered_map<uint16_t, std::function<Response(Connection&, RequestCodes)>>;

	Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port);

	void addRequestHandler(RequestCodes code, std::function<Response(Connection&, RequestCodes)> handler);
	Response dispatch(RequestCodes code);
	void send(Request& req);
	Response recvResponse();

private:
	header_t readHeader();
	bytes_t readPayload(const header_t& header);
	size_t recv(bytes_t& outBytes, size_t recvSz);

private:
	io_ctx_t& m_ctx;
	resolver_t m_resolver;
	socket_t m_socket;

	handler_map_t m_handlerMap;
};

