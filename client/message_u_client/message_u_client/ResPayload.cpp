#include "ResPayload.h"
#include "Response.h"
#include "Utils.h"
#include "Config.h"

#include <stdexcept>
#include <string>

ResPayload::payload_t ResPayload::fromBytes(const bytes_t& bytes, ResponseCodes code)
{
	switch (code)
	{
	case ResponseCodes::REG_OK:
		return std::make_unique<RegistrationOkPayload>(bytes);
	default:
		throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
	}
}

RegistrationOkPayload::RegistrationOkPayload(const bytes_t& bytes)
	: m_uuid{}
{
	ptrdiff_t diff = bytes.end() - bytes.begin();
	if (diff == 0 || diff > Config::CLIENT_ID_SZ) {
		throw std::runtime_error("Error: Received an invalid user id");
	}

	m_uuid.resize(diff);
	std::copy(bytes.begin(), bytes.end(), m_uuid.begin());
}

const std::string& RegistrationOkPayload::getUUID() const
{
	return m_uuid;
}

const ResponseCodes RegistrationOkPayload::getResCode() const
{
	return ResponseCodes::REG_OK;
}
