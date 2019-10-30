#pragma once

namespace Lobster {
	namespace StringOps {
		std::vector<std::string> split(std::string str, char delimiter);
		std::string substr(const std::string& str, const char* from, const char* to);
		std::vector<std::string> RegexAllOccurrence(const std::string& str, const char* expr);
		void Erase(std::string& str, const std::string& delimiters);
		std::string Trim(const std::string& str);
	}
}
