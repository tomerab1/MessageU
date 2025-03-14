#pragma once

#include <vector>
#include <cstdint>
#include <fstream>
#include <string>

// Forward declarations for the message types and request codes enums
enum class MessageTypes : uint8_t;
enum class RequestCodes : uint16_t;

// Base class for the request payloads
class ReqPayload {
public:
	using bytes_t = std::vector<uint8_t>;

	// Converts the payload to a byte array
	virtual bytes_t toBytes() = 0;

	// Returns the size of the payload in bytes
	virtual uint32_t getSize() = 0;
};

// Request payload for the register request
class RegisterReqPayload : public ReqPayload
{
public:
	using name_t = std::string;
	using pub_key_t = std::string;

	RegisterReqPayload(const name_t& name, const pub_key_t& pubKey);

	bytes_t toBytes() override;
	uint32_t getSize() override;

private:
	name_t m_name;
	pub_key_t m_pubKey;
};

// Request payload for the login request
class UsersListReqPayload : public ReqPayload {
public:
	bytes_t toBytes() override;
	uint32_t getSize() override;
};

// Request payload for the get public key request
class GetPublicKeyReqPayload : public ReqPayload {
public:
	GetPublicKeyReqPayload(const std::string& targetId);

	bytes_t toBytes() override;
	uint32_t getSize() override;

private:
	std::string m_targetId;
};

// Request payload for the send message request
class SendMessageReqPayload : public ReqPayload {
public:
	SendMessageReqPayload(const std::string& targetId, MessageTypes type, uint32_t msgSz, const std::string& msg);

	bytes_t toBytes() override;
	uint32_t getSize() override;

private:
	std::string m_targetId;
	MessageTypes m_type;
	uint32_t m_msgSz;
	std::string m_msg;
};

// Request payload for the poll messages request
class PollMessagesReqPayload : public ReqPayload
{
public:
	bytes_t toBytes() override;
	uint32_t getSize() override;
};