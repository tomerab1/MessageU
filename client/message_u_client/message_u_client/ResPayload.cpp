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
		return std::make_unique<RegistrationResPayload>(bytes);
	case ResponseCodes::USRS_LIST:
		return std::make_unique<UsersListResPayload>(bytes);
	case ResponseCodes::PUB_KEY:
		return std::make_unique<PublicKeyResPayload>(bytes);
	case ResponseCodes::MSG_SEND:
		break;
	case ResponseCodes::POLL_MSGS:
		break;
	case ResponseCodes::ERR: 
		break;
	default:
		throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
	}
}

RegistrationResPayload::RegistrationResPayload(const bytes_t& bytes)
	: m_uuid{}
{
	ptrdiff_t diff = bytes.end() - bytes.begin();
	if (diff == 0 || diff > Config::CLIENT_ID_SZ) {
		throw std::runtime_error("Error: Received an invalid user id");
	}

	m_uuid.resize(diff);
	std::copy(bytes.begin(), bytes.end(), m_uuid.begin());
}

const std::string& RegistrationResPayload::getUUID() const
{
	return m_uuid;
}

const ResponseCodes RegistrationResPayload::getResCode() const
{
	return ResponseCodes::REG_OK;
}

UsersListResPayload::UsersListResPayload(const bytes_t& bytes)
{
	if (bytes.empty()) {
		throw std::runtime_error("Error: UsersListResPayload payload is empty");
	}

	size_t numUsers = bytes.size() / (Config::CLIENT_ID_SZ + Config::NAME_MAX_SZ);
	size_t offset{ 0 };
	m_users.resize(numUsers);

	for (size_t i = 0; i < numUsers; i++) {
		UserEntry& curr = m_users[i];
		curr.id.resize(Config::CLIENT_ID_SZ);
		curr.name.resize(Config::NAME_MAX_SZ);

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::CLIENT_ID_SZ, curr.id.begin());
		offset += Config::CLIENT_ID_SZ;

		std::copy(bytes.begin() + offset, bytes.begin() + offset + Config::NAME_MAX_SZ, curr.name.begin());
		offset += Config::NAME_MAX_SZ;

		curr.name.shrink_to_fit();
	}
}

const ResponseCodes UsersListResPayload::getResCode() const
{
	return ResponseCodes::USRS_LIST;
}

const std::vector<UsersListResPayload::UserEntry>& UsersListResPayload::getUsers() const
{
	return m_users;
}

PublicKeyResPayload::PublicKeyResPayload(const bytes_t& bytes)
{
	if (bytes.empty()) {
		throw std::runtime_error("Error: PublicKeyPayload payload is empty");
	}

	m_entry.id.resize(Config::CLIENT_ID_SZ);
	m_entry.pubKey.resize(Config::PUB_KEY_SZ);

	std::copy(bytes.begin(), bytes.begin() + Config::CLIENT_ID_SZ, m_entry.id.begin());
	std::copy(bytes.begin(), bytes.begin() + Config::PUB_KEY_SZ, m_entry.pubKey.begin());
}

const ResponseCodes PublicKeyResPayload::getResCode() const
{
	return ResponseCodes::PUB_KEY;
}

const PublicKeyResPayload::PublicKeyEntry& PublicKeyResPayload::getPubKeyEntry() const
{
	return m_entry;
}
