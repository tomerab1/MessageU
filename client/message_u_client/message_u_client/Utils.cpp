#include "Utils.h"

#include "AESWrapper.h"

namespace Utils {
	int32_t strToInt(std::string& str) {
		return std::stoi(str);
	}

	void trimStr(std::string& str) {
		// erase spaces from the beginning of the string
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));

		// erase spaces from the end of the string
		str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), str.end());
	}
}