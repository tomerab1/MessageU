#include "Utils.h"

namespace Utils {
	int32_t strToInt(std::string& str) {
		std::size_t len{};

		int32_t val{ std::stoi(str, &len) };
		return (len == str.size()) ? val : 0.f;
	}

	void trimStr(std::string& str) {
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));

		str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), str.end());
	}
}