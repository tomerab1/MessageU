#pragma once
#include "Payload.h"
#include "Config.h"

#include <string>

class RegisterPayload : public Payload
{
public:
	using name_t = std::string;
	using pub_key_t = std::string;

	RegisterPayload(const name_t& name, const pub_key_t& pubKey);

	bytes_t toBytes() override;
	uint32_t getSize() override;

private:
	name_t m_name;
	pub_key_t m_pubKey;
};

