#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <filesystem>
#include <boost/endian/conversion.hpp>

namespace Utils {

	/**
	 * Serializes a trivial type into a vector of bytes
	 */
	template<typename T>
	void serializeTrivialType(std::vector<uint8_t>& outVec, size_t& outOffset, T toSerialize) {
		auto inNetOrder = boost::endian::native_to_little(toSerialize);
		std::memcpy(outVec.data() + outOffset, &toSerialize, sizeof(T));
		outOffset += sizeof(T);
	}

	/**
	 * Deserializes a trivial type from a vector of bytes
	 */
	template<typename T>
	T deserializeTrivialType(const std::vector<uint8_t>& bytes, size_t& outOffset) {
		T res;
		std::memcpy(&res, bytes.data() + outOffset, sizeof(T));
		outOffset += sizeof(T);
		return res;
	}

	/**
	 * Converts an enum type to an uint16_t
	 */
	template<typename T>
	uint16_t EnumToUint16(T enumVal) {
		return static_cast<std::underlying_type_t<T>>(enumVal);
	}

	/**
	 * Converts an enum type to an uint8_t
	 */
	template<typename T>
	uint8_t EnumToUint8(T enumVal) {
		return static_cast<std::underlying_type_t<T>>(enumVal);
	}

	/**
	 * Converts a string to an uint32_t
	 */
	int32_t strToInt(std::string& str);

	/**
	 * Trims a string on both ends.
	 */
	void trimStr(std::string& str);
	
	/**
	 * Generates a unique file path
	 */
	std::filesystem::path getUniquePath(uint32_t msgId);
}
