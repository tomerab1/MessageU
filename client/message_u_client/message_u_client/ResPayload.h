#pragma once

#include <memory>
#include <vector>
#include <string>

enum class ResponseCodes : uint16_t;

class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	static payload_t fromBytes(const bytes_t& bytes, ResponseCodes code);

	virtual const ResponseCodes getResCode() const = 0;

	virtual ~ResPayload() = default;
};

class RegistrationResPayload : public ResPayload {
public:
	RegistrationResPayload(const bytes_t& bytes);

	const std::string& getUUID() const;
	const ResponseCodes getResCode() const override;

	~RegistrationResPayload() = default;

private:
	std::string m_uuid;
};

class UsersListResPayload : public ResPayload {
public:
	UsersListResPayload(const bytes_t& bytes);

	struct UserEntry {
		std::string id;
		std::string name;
	};

	const ResponseCodes getResCode() const override;
	const std::vector<UserEntry>& getUsers() const;

private:
	std::vector<UserEntry> m_users;
};

class PublicKeyPayload : public ResPayload {};

class MessageSentPayload : public ResPayload {};

class PollMessagePayload : public ResPayload {};

class ErrorPayload : public ResPayload {};