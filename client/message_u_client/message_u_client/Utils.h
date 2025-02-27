#pragma once

#include <vector>
#include <cstdint>
#include <boost/endian/conversion.hpp>

namespace Utils {
	template<typename T>
	void serializeTrivialType(std::vector<uint8_t>& outVec, size_t& outOffset, T toSerialize) {
		auto inNetOrder = boost::endian::native_to_big(toSerialize);
		std::memcpy(outVec.data() + outOffset, &inNetOrder, sizeof(T));
		outOffset += sizeof(T);
	}

	template<typename T>
	T deserializeTrivialType(const std::vector<uint8_t>& bytes, size_t& outOffset) {
		T res;
		std::memcpy(&res, bytes.data() + outOffset, sizeof(T));
		outOffset += sizeof(T);
		return boost::endian::big_to_native(res);
	}

	template<typename T>
	uint16_t EnumToUint16(T enumVal) {
		return static_cast<std::underlying_type_t<T>>(enumVal);
	}
}