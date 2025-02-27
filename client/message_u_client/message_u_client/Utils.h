#pragma once

#include <vector>
#include <cstdint>
#include <boost/endian/conversion.hpp>

namespace Utils {
	template<typename T>
	void serializeTrivialType(std::vector<uint8_t>& outVec, size_t& outOffset, const T& toSerialize) {
		auto inNetOrder = boost::endian::native_to_big(toSerialize);
		std::memcpy(outVec.data() + outOffset, &inNetOrder, sizeof(T));
		outOffset += sizeof(T);
	}
}