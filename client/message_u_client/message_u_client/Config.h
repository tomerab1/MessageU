#pragma once

#include <cstdint>

namespace Config {
	static constexpr uint8_t VERSION = 1;
	static constexpr uint8_t HEADER_BYTES_SZ = 23;
	static constexpr uint8_t NAME_MAX_SZ = 255;
	static constexpr uint8_t PUB_KEY_SZ = 160;
	static constexpr size_t CLIENT_ID_SZ = 16;
	static constexpr size_t RES_HEADER_SZ = 7;
}