#pragma once

#include <vector>
#include <cstdint>
#include <string>

enum class MessageTypes : uint8_t;
enum class RequestCodes : uint16_t;

class ReqPayload {
public:
	using bytes_t = std::vector<uint8_t>;

	virtual bytes_t toBytes() = 0;
	virtual uint32_t getSize() = 0;
};

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

class UsersListReqPayload : public ReqPayload {
public:
	bytes_t toBytes() override;
	uint32_t getSize() override;
};

class GetPublicKeyReqPayload : public ReqPayload {
public:
	GetPublicKeyReqPayload(const std::string& targetId);

	bytes_t toBytes() override;
	uint32_t getSize() override;

private:
	std::string m_targetId;
};

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

class PollMessagesReqPayload : public ReqPayload
{
public:
	bytes_t toBytes() override;
	uint32_t getSize() override;
};