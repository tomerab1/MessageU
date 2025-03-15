#pragma once

#include <cstdint>
#include <string>

namespace Config {
	static constexpr uint8_t VERSION = 2; // Client version
	static constexpr uint8_t HEADER_BYTES_SZ = 23; // Number of bytes in the requet header
	static constexpr uint8_t NAME_MAX_SZ = 255; // Maximum size of a client name
	static constexpr uint8_t PUB_KEY_SZ = 160; // Size of the public key
	static constexpr size_t CLIENT_ID_SZ = 16; // Size of the client ID
	static constexpr size_t RES_HEADER_SZ = 7; // Number of bytes in the response header
	static constexpr size_t CHUNK_SZ = 1024; // Chunk size for the socket buffer 
	static constexpr const char* ME_DOT_INFO_PATH = "./me.info"; // Path of the client info file
	static const std::string EMPTY_UUID = ""; // Empty UUID

	static const std::string SERVER_ADDR = "localhost"; // Server address
	static const std::string SERVER_PORT = "1234"; // Server port
}