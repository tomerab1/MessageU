#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <boost/asio.hpp>

#include "Response.h"

enum class RequestCodes : uint16_t;

class ResPayload;
class Request;

class HeaderValidator {
public:
	HeaderValidator();

	void setReqCode(RequestCodes code);
	bool accept(const std::vector<uint8_t>& bytes);
	std::string what();

	struct MapEntry {
		MapEntry(const std::vector<ResponseCodes>& codes, const std::vector<std::optional<uint32_t>>& expectedSzs);

		std::vector<ResponseCodes> expectedCodes;
		std::vector<std::optional<uint32_t>> expectedSzs;
	};

private:
	RequestCodes m_reqCode;
	std::string m_err;
	std::unordered_map<RequestCodes, MapEntry> m_reqCodeToExpectedRes;
};

class PayloadValidator {
public:
	bool accept(const std::vector<uint8_t>& bytes);
	std::string what();
};

class Connection
{
public:
	using io_ctx_t = boost::asio::io_context;
	using resolver_t = boost::asio::ip::tcp::resolver;
	using socket_t = boost::asio::ip::tcp::socket;
	using header_t = Response::Header;
	using bytes_t = std::vector<uint8_t>;

	Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port);
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

	HeaderValidator m_headerValidator;
	PayloadValidator m_payloadValidator;
};