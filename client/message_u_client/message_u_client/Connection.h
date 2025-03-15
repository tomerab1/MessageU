#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <functional>
#include <filesystem>
#include <boost/asio.hpp>

#include "Response.h"
#include "Request.h"

// Class that validates the header of a response
class HeaderValidator {
public:
	HeaderValidator();

	// Set the current request code
	void setReqCode(RequestCodes code);

	// Validate the header
	void validate(const std::vector<uint8_t>& bytes);

	// Maps a response codes to the expected response codes and sizes
	struct MapEntry {
		MapEntry(const std::vector<ResponseCodes>& codes, const std::vector<std::optional<uint32_t>>& expectedSzs);

		std::vector<ResponseCodes> expectedCodes;
		std::vector<std::optional<uint32_t>> expectedSzs;
	};

private:
	RequestCodes m_reqCode;
	std::unordered_map<RequestCodes, MapEntry> m_reqCodeToExpectedRes;
};

class PayloadValidator {
public:
	using header_t = Response::Header;

	void validate(const header_t& header, const std::vector<uint8_t>& bytes);
};

// Class for wrapping the connection to the server
class Connection
{
public:
	// Aliases 
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