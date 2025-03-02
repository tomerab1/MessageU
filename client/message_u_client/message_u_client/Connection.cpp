#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "Utils.h"
#include "Config.h"

Connection::Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port)
	: m_ctx{ctx}, m_socket{ ctx }, m_resolver{ctx}
{
	boost::asio::connect(m_socket, m_resolver.resolve(addr, port));
}

void Connection::addHandler(RequestCodes code, std::function<Response(Connection&, RequestCodes)> handler)
{
	if (m_handlerMap.find(Utils::EnumToUint16(code)) != m_handlerMap.end()) {
		return;
	}

	m_handlerMap.insert({ Utils::EnumToUint16(code), handler });
}

Response Connection::dispatch(RequestCodes code)
{
	return m_handlerMap[Utils::EnumToUint16(code)](*this, code);
}

Connection::header_t Connection::readHeader()
{
	std::vector<uint8_t> headerBytes(Config::RES_HEADER_SZ, 0);
	recv(headerBytes, Config::RES_HEADER_SZ);
	return Response::Header::fromBytes(headerBytes);
}

Connection::bytes_t Connection::readPayload(const header_t& header)
{
	std::vector<uint8_t> payloadBytes(header.payloadSz, 0);
	recv(payloadBytes, header.payloadSz);
	return payloadBytes;
}

void Connection::send(Request& req)
{
	auto bytes = req.toBytes();
	boost::asio::write(m_socket, boost::asio::buffer(bytes.data(), bytes.size()));
}

Response Connection::recvResponse()
{
	auto header = readHeader();
	auto payloadBytes = readPayload(header);
	return Response(header, payloadBytes);
}

size_t Connection::recv(bytes_t& outBytes, size_t recvSz)
{
	return boost::asio::read(m_socket, boost::asio::buffer(outBytes.data(), recvSz));
}
