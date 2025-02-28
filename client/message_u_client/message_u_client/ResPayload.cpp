#include "ResPayload.h"
#include "Response.h"
#include "Utils.h"

#include <stdexcept>
#include <string>

ResPayload::payload_t ResPayload::fromBytes(const bytes_t& bytes, ResponseCodes code)
{
	switch (code)
	{
	case ResponseCodes::REG_OK:
		break;
	default:
		throw std::runtime_error("Error: '" + std::to_string(Utils::EnumToUint16(code)) + "' is not a valid code");
	}
}
