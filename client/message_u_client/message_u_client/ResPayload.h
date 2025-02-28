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

	virtual const std::string& getUUID() const = 0;
	virtual const ResponseCodes getResCode() const = 0;

	virtual ~ResPayload() {}
};

class RegistrationOkPayload : public ResPayload {
public:
	RegistrationOkPayload(const bytes_t& bytes);

	const std::string& getUUID() const override;
	const ResponseCodes getResCode() const override;

	~RegistrationOkPayload() {}

private:
	std::string m_uuid;
};