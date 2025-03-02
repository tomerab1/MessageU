#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/asio.hpp>

#include "Response.h"

enum class RequestCodes : uint16_t;
class ResPayload;
class Request;

class Middleware {
public:
	virtual bool accept(const Response::Header& header) = 0;
	virtual bool accept(const ResPayload& payload) = 0;
};

class Connection
{
public:
	using io_ctx_t = boost::asio::io_context;
	using resolver_t = boost::asio::ip::tcp::resolver;
	using socket_t = boost::asio::ip::tcp::socket;
	using header_t = Response::Header;
	using bytes_t = std::vector<uint8_t>;
	using handler_map_t = std::unordered_map<uint16_t, std::function<Response(Connection&, RequestCodes)>>;
	using middleware_t = std::unique_ptr<Middleware>;

	Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port);

	void addHandler(RequestCodes code, std::function<Response(Connection&, RequestCodes)> handler);
	Response dispatch(RequestCodes code);
	void send(Request& req);
	Response recvResponse();

	void addHeaderMiddleware(const Middleware& middleware);
	void addResponseMiddleware(const Middleware& middleware);

private:
	header_t readHeader();
	bytes_t readPayload(const header_t& header);
	size_t recv(bytes_t& outBytes, size_t recvSz);

private:
	io_ctx_t& m_ctx;
	resolver_t m_resolver;
	socket_t m_socket;

	handler_map_t m_handlerMap;
	std::vector<middleware_t> m_header_middleware;
	std::vector<middleware_t> m_payload_middleware;
};

