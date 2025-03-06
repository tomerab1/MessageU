#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "Utils.h"
#include "Config.h"

#include <boost/range/combine.hpp>
#include <string>

Connection::Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port)
	: m_ctx{ ctx }, m_socket{ ctx }, m_resolver{ ctx }
{
	boost::asio::connect(m_socket, m_resolver.resolve(addr, port));
}

Connection::header_t Connection::readHeader()
{
	std::vector<uint8_t> headerBytes;
	headerBytes.resize(Config::RES_HEADER_SZ);

	recv(headerBytes, Config::RES_HEADER_SZ);
	m_headerValidator.validate(headerBytes);

	return Response::Header::fromBytes(headerBytes);
}

Connection::bytes_t Connection::readPayload(const header_t& header)
{
	std::vector<uint8_t> payloadBytes;
	payloadBytes.resize(header.payloadSz);

	recv(payloadBytes, header.payloadSz);
	m_payloadValidator.validate(payloadBytes);

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
	size_t offset{ 0 };
	while (offset < recvSz) {
		auto readSz = std::min(recvSz, Config::RECV_SZ);
		auto bytesRead = boost::asio::read(m_socket, boost::asio::buffer(outBytes.data() + offset, readSz));
		offset += bytesRead;
	}

	return offset;
}

HeaderValidator::MapEntry::MapEntry(const std::vector<ResponseCodes>& codes, const std::vector<std::optional<uint32_t>>& expectedSzs)
	: expectedCodes{ codes }, expectedSzs{ expectedSzs }
{
}

HeaderValidator::HeaderValidator()
	: m_reqCode{}
{
	m_reqCodeToExpectedRes.insert({ RequestCodes::REGISTER, {{ResponseCodes::REG_OK, ResponseCodes::ERR}, {Config::CLIENT_ID_SZ, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::USRS_LIST,  {{ResponseCodes::USRS_LIST, ResponseCodes::ERR}, {std::nullopt, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::GET_PUB_KEY, {{ResponseCodes::PUB_KEY, ResponseCodes::ERR}, {std::nullopt, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::SEND_MSG,  {{ResponseCodes::MSG_SEND, ResponseCodes::ERR}, {std::nullopt, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::POLL_MSGS, {{ResponseCodes::POLL_MSGS, ResponseCodes::ERR}, {std::nullopt, 0}} });
}

void HeaderValidator::setReqCode(RequestCodes code)
{
	m_reqCode = code;
}

void HeaderValidator::validate(const std::vector<uint8_t>& bytes)
{
	auto itr = m_reqCodeToExpectedRes.find(m_reqCode);
	if (itr == m_reqCodeToExpectedRes.end()) {
		throw std::runtime_error("Error: Unexpected request code '" + std::to_string(Utils::EnumToUint16(m_reqCode)) + "'");
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
		else if (ResponseCodes(code) == expectedCode && !expectedSz.has_value()) {
			isValid = true;
			break;
		}
	}

	if (!isValid) {
		throw std::runtime_error("Error: Unexpected response combination: code " + std::to_string(code) + " with payload size " + std::to_string(payloadSz));
	}
}

void PayloadValidator::validate(const std::vector<uint8_t>& bytes)
{
	
}
