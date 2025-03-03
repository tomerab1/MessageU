#pragma once

#include <memory>
#include <vector>
#include <string>

class Visitor;

enum class ResponseCodes : uint16_t;
enum class MessageTypes : uint8_t;

class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	static payload_t fromBytes(const bytes_t& bytes, ResponseCodes code);

	virtual void visit(Visitor& visitor) = 0;
	virtual std::string toString() const = 0;

	virtual ~ResPayload() = default;
};

class RegistrationResPayload : public ResPayload {
public:
	RegistrationResPayload(const bytes_t& bytes);

	void visit(Visitor& visitor) override;
	std::string toString() const override;
	const std::string& getUUID() const;

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

	void visit(Visitor& visitor) override;
	std::string toString() const override;
	const std::vector<UserEntry>& getUsers() const;

private:
	std::vector<UserEntry> m_users;
};

class PublicKeyResPayload : public ResPayload {
public:
	PublicKeyResPayload(const bytes_t& bytes);

	struct PublicKeyEntry {
		std::string id;
		std::string pubKey;
	};

	void visit(Visitor& visitor) override;
	std::string toString() const override;
	const PublicKeyEntry& getPubKeyEntry() const;

private:
	PublicKeyEntry  m_entry;
};

class MessageSentResPayload : public ResPayload {
public:
	MessageSentResPayload(const bytes_t& bytes);

	struct MsgEntry {
		std::string targetId;
		uint32_t msgId;
	};

	void visit(Visitor& visitor) override;
	std::string toString() const override;

private:
	MsgEntry m_entry;
};

class PollMessageResPayload : public ResPayload {
public:
	PollMessageResPayload(const bytes_t& bytes);

	struct MessageEntry {
		std::string senderId;
		uint32_t msgId;
		MessageTypes msgType;
		uint32_t contentSz;
		std::string content;
	};

	void visit(Visitor& visitor) override;
	std::string toString() const override;

private:
	std::vector<MessageEntry> m_msgs;
};

class ErrorPayload : public ResPayload {
public:
	void visit(Visitor& visitor) override;
	std::string toString() const override;
};

class Visitor 
{
public:
	virtual void accept(const RegistrationResPayload& payload) = 0;
	virtual void accept(const UsersListResPayload& payload) = 0;
	virtual void accept(const PublicKeyResPayload& payload) = 0;
	virtual void accept(const MessageSentResPayload& payload) = 0;
	virtual void accept(const PollMessageResPayload& payload) = 0;
	virtual void accept(const ErrorPayload& payload) = 0;
};

class ToStringVisitor : public Visitor {
public:
};

class ClientStateVisitor : public Visitor {
public:
};