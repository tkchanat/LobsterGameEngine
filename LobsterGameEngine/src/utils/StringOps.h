#pragma once
#include <glm/vec3.hpp>

namespace Lobster {
	//	Global static StringOps functions. Call by StringOps::func_name().
	namespace StringOps {
		//	Split a string into a vector of strings by delimeter.
		std::vector<std::string> split(std::string str, char delimiter);

		//	Returning a substring by char* with nullptr handling.
		std::string substr(const std::string& str, const char* from, const char* to);

		//  Wrapper for std::regex_replace
		std::string RegexReplace(const std::string& str, const char* expr, const char* pattern);
		inline std::string GetValidFilename(const std::string& str) { return RegexReplace(str, "[,.<>:\"/\\|?*]", ""); }
		
		//	Finding occurrence of expression with regex.
		std::vector<std::string> RegexAllOccurrence(const std::string& str, const char* expr);

		//	Inplace eradication of all occurrence of given string delimeter.
		void Erase(std::string& str, const std::string& delimiters);

		//	Inplace trim of whitespaces.
		std::string Trim(const std::string& str);

		//	Inplace replacement of a certain delimeter.
		void ReplaceAll(std::string& str, const std::string& from, const std::string& to);

		//	String representation of a glm::vec3 in <x,y,z> form.
		std::string ToString(glm::vec3 vec, int decimals = 2);

		//	String representation of a glm::vec3 in (RGB) form.
		std::string ToColorString(glm::vec3 vec);

		//	String representation of a float in degree (x deg) form.
		std::string ToDegreeString(float val);

		//	String representation of a float with decimal precision cap.
		std::string ToString(float val, int decimals = 3);
	}
}
