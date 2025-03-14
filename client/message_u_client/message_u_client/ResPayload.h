#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sstream>

// Foward declarations, for the client state and the visitor classes
class Visitor;
class ClientState;

// Forward declarations for the response codes and message types enums
enum class ResponseCodes : uint16_t;
enum class MessageTypes : uint8_t;

// Base class for the response payloads
class ResPayload {
public:
	using payload_t = std::unique_ptr<ResPayload>;
	using bytes_t = std::vector<uint8_t>;

	// Converts the byte array to a payload object
	static payload_t fromBytes(const bytes_t& bytes, ResponseCodes code);

	// Accepts a visitor
	virtual void accept(Visitor& visitor) = 0;

	virtual ~ResPayload() = default;
};

// Class to represent the registration response payload
class RegistrationResPayload : public ResPayload {
public:
	RegistrationResPayload(const bytes_t& bytes);

	void accept(Visitor& visitor) override;
	const std::string& getUUID() const;

	~RegistrationResPayload() = default;

private:
	std::string m_uuid;
};

// Class to represent the users list response payload
class UsersListResPayload : public ResPayload {
public:
	UsersListResPayload(const bytes_t& bytes);

	// Entry for each user in the user list
	struct UserEntry {
		std::string id;
		std::string name;
	};

	void accept(Visitor& visitor) override;
	const std::vector<UserEntry>& getUsers() const;

	~UsersListResPayload() = default;

private:
	std::vector<UserEntry> m_users;
};

// Class to represent the public key response payload
class PublicKeyResPayload : public ResPayload {
public:
	PublicKeyResPayload(const bytes_t& bytes);

	// Entry for the parsed payload
	struct PublicKeyEntry {
		std::string id;
		std::string pubKey;
	};

	void accept(Visitor& visitor) override;
	const PublicKeyEntry& getPubKeyEntry() const;

	~PublicKeyResPayload() = default;

private:
	PublicKeyEntry  m_entry;
};

// Class to represent the message sent response payload
class MessageSentResPayload : public ResPayload {
public:
	MessageSentResPayload(const bytes_t& bytes);

	// Entry for the parsed payload
	struct MsgEntry {
		std::string targetId;
		uint32_t msgId{};
	};

	const MsgEntry& getMessage() const;
	void accept(Visitor& visitor) override;

	~MessageSentResPayload() = default;

private:
	MsgEntry m_entry;
};

// Class to represent the poll message response payload
class PollMessageResPayload : public ResPayload {
public:
	PollMessageResPayload(const bytes_t& bytes);

	// Entry for each message in the message list
	struct MessageEntry {
		std::string senderId;
		uint32_t msgId{};
		MessageTypes msgType;
		uint32_t contentSz{};
		std::string content;
	};

	const std::vector<MessageEntry>& getMessages() const;
	void accept(Visitor& visitor) override;

	~PollMessageResPayload() = default;

private:
	std::vector<MessageEntry> m_msgs;
};

// Class to represent the error response payload
class ErrorPayload : public ResPayload {
public:
	void accept(Visitor& visitor) override;

	~ErrorPayload() = default;
};

// Visitor class to visit the response payloads
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

// Visitor class to convert the response payloads to string
class ToStringVisitor : public Visitor {
public:
	explicit ToStringVisitor(ClientState& state);

	std::string getString();

	void visit(const RegistrationResPayload& payload) override;
	void visit(const UsersListResPayload& payload) override;
	void visit(const PublicKeyResPayload& payload) override;
	void visit(const MessageSentResPayload& payload) override;
	void visit(const PollMessageResPayload& payload) override;
	void visit(const ErrorPayload& payload) override;

private:
	ClientState& m_state; // Reference to the client state, may use it for getting a clients info
	std::stringstream m_ss;
};

// Visitor class to update the client state
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
	ClientState& m_state; // Reference to the client state, updates it
};