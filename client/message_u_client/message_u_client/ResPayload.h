#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sstream>

class Visitor;
class ClientState;

enum class ResponseCodes : uint16_t;
enum class MessageTypes : uint8_t;

class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	static payload_t fromBytes(const bytes_t& bytes, ResponseCodes code);

	virtual void accept(Visitor& visitor) = 0;
	virtual ~ResPayload() = default;
};

class RegistrationResPayload : public ResPayload {
public:
	RegistrationResPayload(const bytes_t& bytes);

	void accept(Visitor& visitor) override;
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

	void accept(Visitor& visitor) override;
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

	void accept(Visitor& visitor) override;
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

	const MsgEntry& getMessage() const;
	void accept(Visitor& visitor) override;

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

	const std::vector<MessageEntry>& getMessages() const;
	void accept(Visitor& visitor) override;

private:
	std::vector<MessageEntry> m_msgs;
};

class ErrorPayload : public ResPayload {
public:
	void accept(Visitor& visitor) override;
};

class Visitor 
{
public:
	virtual void visit(const RegistrationResPayload& payload) = 0;
	virtual void visit(const UsersListResPayload& payload) = 0;
	virtual void visit(const PublicKeyResPayload& payload) = 0;
	virtual void visit(const MessageSentResPayload& payload) = 0;
	virtual void visit(const PollMessageResPayload& payload) = 0;
	virtual void visit(const ErrorPayload& payload) = 0;
};

class ToStringVisitor : public Visitor {
public:
	std::string getString();

	void visit(const RegistrationResPayload& payload) override;
	void visit(const UsersListResPayload& payload) override;
	void visit(const PublicKeyResPayload& payload) override;
	void visit(const MessageSentResPayload& payload) override;
	void visit(const PollMessageResPayload& payload) override;
	void visit(const ErrorPayload& payload) override;

private:
	std::stringstream m_ss;
};

class ClientStateVisitor : public Visitor {
public:
	explicit ClientStateVisitor(ClientState& state);

	void visit(const RegistrationResPayload& payload) override;
	void visit(const UsersListResPayload& payload) override;
	void visit(const PublicKeyResPayload& payload) override;
	void visit(const MessageSentResPayload& payload) override;
	void visit(const PollMessageResPayload& payload) override;
	void visit(const ErrorPayload& payload) override;

private:
	ClientState& m_state;
};