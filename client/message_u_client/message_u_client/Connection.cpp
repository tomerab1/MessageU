#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "Utils.h"
#include "Config.h"

#include <string>

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

	if (!m_headerValidator.accept(headerBytes)) {
		throw std::runtime_error(m_headerValidator.what());
	}

	recv(headerBytes, Config::RES_HEADER_SZ);
	return Response::Header::fromBytes(headerBytes);
}

Connection::bytes_t Connection::readPayload(const header_t& header)
{
	std::vector<uint8_t> payloadBytes(header.payloadSz, 0);
	recv(payloadBytes, header.payloadSz);

	 
	if (!m_payloadValidator.accept(payloadBytes)) {
			throw std::runtime_error(m_payloadValidator.what());
	}

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

HeaderValidator::MapEntry::MapEntry(ResponseCodes code, std::optional<uint32_t> expectedSz)
	: expectedCode{code}, expectedSz{expectedSz}
{
}

HeaderValidator::HeaderValidator()
	: m_reqCode{}
{
	m_reqCodeToExpectedRes.insert({ RequestCodes::REGISTER, {ResponseCodes::REG_OK, Config::CLIENT_ID_SZ} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::USRS_LIST,  {ResponseCodes::USRS_LIST, std::nullopt} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::GET_PUB_KEY, {ResponseCodes::PUB_KEY, std::nullopt} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::SEND_MSG,  {ResponseCodes::MSG_SEND, std::nullopt} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::POLL_MSGS, {ResponseCodes::POLL_MSGS, std::nullopt} });
}

void HeaderValidator::setReqCode(RequestCodes code)
{
	m_reqCode = code;
}

bool HeaderValidator::accept(const std::vector<uint8_t>& bytes)
{
	auto itr = m_reqCodeToExpectedRes.find(m_reqCode);
	if (itr == m_reqCodeToExpectedRes.end()) {
		return false;
	}

	auto entry = itr->second;
	size_t offset{ 1 };

	auto code = Utils::deserializeTrivialType<uint16_t>(bytes, offset);
	if (ResponseCodes(code) != entry.expectedCode) {
		m_err = "Error: Unexpeced response code: " + std::to_string(code);
		return false;
	}
	
	auto payloadSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);
	if (entry.expectedSz != std::nullopt && payloadSz != entry.expectedSz.value()) {
		m_err = "Error: Expected '" + std::to_string(entry.expectedSz.value()) + "' bytes but received '" + std::to_string(payloadSz) + "' bytes";
		return false;
	}

	return true;
}

std::string HeaderValidator::what()
{
	return m_err;
}

bool PayloadValidator::accept(const std::vector<uint8_t>& bytes)
{
	return false;
}

std::string PayloadValidator::what()
{
	return std::string();
}

