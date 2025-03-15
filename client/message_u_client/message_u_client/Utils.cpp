#include "Utils.h"

#include <chrono>
#include  <stringstream>

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

	std::filesystem::path getUniquePath() {
		auto now = std::chrono::system_clock::now();
		auto timeT = std::chrono::system_clock::to_time_t(now);
		std::stringstream filename_ss;

		filename_ss << "file_" << messages[i].msgId << "_" << timeT;
		std::string filename = filename_ss.str();

		return std::filesystem::temp_directory_path() / filename;
	}
}