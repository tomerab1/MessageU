#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "Utils.h"
#include "Config.h"

#include <boost/range/combine.hpp>
#include <string>

Connection::Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port)
	: m_ctx{ctx}, m_socket{ ctx }, m_resolver{ctx}
{
	boost::asio::connect(m_socket, m_resolver.resolve(addr, port));
}

Connection::header_t Connection::readHeader()
{
	std::vector<uint8_t> headerBytes(Config::RES_HEADER_SZ, 0);
	recv(headerBytes, Config::RES_HEADER_SZ);

	if (!m_headerValidator.accept(headerBytes)) {
		throw std::runtime_error(m_headerValidator.what());
	}

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
	m_headerValidator.setReqCode(req.getCode());
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

HeaderValidator::MapEntry::MapEntry(const std::vector<ResponseCodes>& codes, const std::vector<std::optional<uint32_t>>& expectedSzs)
	: expectedCodes{codes}, expectedSzs{ expectedSzs }
{
}

HeaderValidator::HeaderValidator()
	: m_reqCode{}
{
	m_reqCodeToExpectedRes.insert({ RequestCodes::REGISTER, {{ResponseCodes::REG_OK, ResponseCodes::ERR}, {Config::CLIENT_ID_SZ, 0}}});
	m_reqCodeToExpectedRes.insert({ RequestCodes::USRS_LIST,  {{ResponseCodes::USRS_LIST, ResponseCodes::ERR}, {std::nullopt, 0}}});
	m_reqCodeToExpectedRes.insert({ RequestCodes::GET_PUB_KEY, {{ResponseCodes::PUB_KEY, ResponseCodes::ERR}, {std::nullopt, 0}}});
	m_reqCodeToExpectedRes.insert({ RequestCodes::SEND_MSG,  {{ResponseCodes::MSG_SEND, ResponseCodes::ERR}, {std::nullopt, 0}}});
	m_reqCodeToExpectedRes.insert({ RequestCodes::POLL_MSGS, {{ResponseCodes::POLL_MSGS, ResponseCodes::ERR}, {std::nullopt, 0}}});
}

void HeaderValidator::setReqCode(RequestCodes code)
{
	m_reqCode = code;
}

bool HeaderValidator::accept(const std::vector<uint8_t>& bytes)
{
	auto itr = m_reqCodeToExpectedRes.find(m_reqCode);
	if (itr == m_reqCodeToExpectedRes.end()) {
		m_err = "Error: Unexpected request code '" + std::to_string(Utils::EnumToUint16(m_reqCode)) + "'";
		return false;
	}

	auto entry = itr->second;
	size_t offset{ 1 };

	auto code = Utils::deserializeTrivialType<uint16_t>(bytes, offset);
	auto payloadSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);

	bool isValid{ false };
	for (const auto& [expectedCode, expectedSz] : boost::combine(entry.expectedCodes, entry.expectedSzs)) {
		if (ResponseCodes(code) == expectedCode && payloadSz == expectedSz) {
			isValid = true;
			break;
		}
	}

	if (!isValid) {
		m_err = "Error: Unexpected response combination: code " + std::to_string(code) + " with payload size " + std::to_string(payloadSz);
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
	return true;
}

std::string PayloadValidator::what()
{
	return std::string();
}

