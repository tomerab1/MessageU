#include "Connection.h"
#include "Request.h"
#include "ResPayload.h"
#include "ReqPayload.h"
#include "Utils.h"
#include "Config.h"

#include <boost/range/combine.hpp>
#include <string>

Connection::Connection(io_ctx_t& ctx, const std::string& addr, const std::string& port)
	: m_ctx{ ctx }, m_socket{ ctx }, m_resolver{ ctx }
{
	boost::asio::connect(m_socket, m_resolver.resolve(addr, port));
}

// Reads the header of a response and parses it to be a Response::Header object
Connection::header_t Connection::readHeader()
{
	std::vector<uint8_t> headerBytes;
	headerBytes.resize(Config::RES_HEADER_SZ);

	recv(headerBytes, Config::RES_HEADER_SZ);

	// Validate the header
	m_headerValidator.validate(headerBytes);

	return Response::Header::fromBytes(headerBytes);
}

// Reads the payload of a response and returns it as a vector of bytes
Connection::bytes_t Connection::readPayload(const header_t& header)
{
	std::vector<uint8_t> payloadBytes;
	payloadBytes.resize(header.payloadSz);

	recv(payloadBytes, header.payloadSz);
	// Validate the payload
	m_payloadValidator.validate(header, payloadBytes);

	return payloadBytes;
}

// Sends a request to the server
void Connection::send(Request& req)
{
	// Sets the current request code that is being sent, so we can later use it to validate the servers response
	m_headerValidator.setReqCode(req.getCode());
	// Convert the req to bytes and send it
	auto bytes = req.toBytes();
	boost::asio::write(m_socket, boost::asio::buffer(bytes.data(), bytes.size()));
}

// Receives a response from the server, returns a Response object
Response Connection::recvResponse()
{
	auto header = readHeader();
	auto payloadBytes = readPayload(header);
	return Response(header, payloadBytes);
}

// Reads bytes from the server, stores the bytes in outBytes and returns the number of bytes read
size_t Connection::recv(bytes_t& outBytes, size_t recvSz)
{
	size_t offset{ 0 };
	while (offset < recvSz) {
		// Read at most CHUNK_SZ at a time
		auto readSz = std::min(recvSz - offset, Config::CHUNK_SZ);
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
	// Initialize the map with the expected response codes and sizes for each request code
	// std::nullopt means that the payload is of variable size
	m_reqCodeToExpectedRes.insert({ RequestCodes::REGISTER, {{ResponseCodes::REG_OK, ResponseCodes::ERR}, {Config::CLIENT_ID_SZ, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::USRS_LIST,  {{ResponseCodes::USRS_LIST, ResponseCodes::ERR}, {std::nullopt, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::GET_PUB_KEY, {{ResponseCodes::PUB_KEY, ResponseCodes::ERR}, {Config::CLIENT_ID_SZ + Config::PUB_KEY_SZ, 0}} });
	m_reqCodeToExpectedRes.insert({ RequestCodes::SEND_MSG,  {{ResponseCodes::MSG_SEND, ResponseCodes::ERR}, {Config::CLIENT_ID_SZ + sizeof(uint32_t), 0}}});
	m_reqCodeToExpectedRes.insert({ RequestCodes::POLL_MSGS, {{ResponseCodes::POLL_MSGS, ResponseCodes::ERR}, {std::nullopt, 0}} });
}

void HeaderValidator::setReqCode(RequestCodes code)
{
	m_reqCode = code;
}

void HeaderValidator::validate(const std::vector<uint8_t>& bytes)
{
	// Check if the request code is in the map, if not, throw an error
	auto itr = m_reqCodeToExpectedRes.find(m_reqCode);
	if (itr == m_reqCodeToExpectedRes.end()) {
		throw std::runtime_error("Error: Unexpected request code '" + std::to_string(Utils::EnumToUint16(m_reqCode)) + "'");
	}

	// Get the expected response codes and sizes for the current request code
	auto entry = itr->second;
	size_t offset{ 1 };

	// Get the response code and payload size
	auto code = Utils::deserializeTrivialType<uint16_t>(bytes, offset);
	auto payloadSz = Utils::deserializeTrivialType<uint32_t>(bytes, offset);

	bool isValid{ false };
	// Iterate over the expected response codes and sizes and check if the response code and payload size match any of them
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

	// If its invalid, throw a runtime error
	if (!isValid) {
		throw std::runtime_error("Error: Unexpected response combination: code " + std::to_string(code) + " with payload size " + std::to_string(payloadSz));
	}
}

void PayloadValidator::validate(const header_t& header, const std::vector<uint8_t>& bytes)
{
    // Makes sure that the payload size matches the size declared in the header
    if (bytes.size() != header.payloadSz) {
        throw std::runtime_error("Error: Payload size (" + std::to_string(bytes.size()) +
                                 ") does not match header's declared size (" + std::to_string(header.payloadSz) + ").");
    }
}
