#include "pch.h"
#include "StringOps.h"

namespace Lobster
{
	namespace StringOps
	{
		std::vector<std::string> split(std::string str, char delimiter) {
			std::vector<std::string> splitStr;
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, delimiter)) {
				splitStr.push_back(token);
			}
			return splitStr;
		}

		std::string substr(const std::string& str, const char* from, const char* to) {
			size_t begin = from == nullptr ? 0 : str.find(from);
			size_t end = to == nullptr ? std::string::npos : str.find(to);
			return str.substr(begin, end);
		}

		std::string RegexReplace(const std::string & str, const char * expr, const char * pattern)
		{
			const std::regex r(expr);
			return std::regex_replace(str, r, pattern);
		}

		std::vector<std::string> RegexAllOccurrence(const std::string& str, const char* expr) {
			std::vector<std::string> output;
			// extracting the matching pattern.
			std::string subject = str;
			std::smatch match;
			const std::regex r(expr);
			int i = 1;
			while (std::regex_search(subject, match, r)) {
				output.emplace_back(match.str());
				subject = match.suffix().str();
			}
			return output;
		}

		void Erase(std::string& str, const std::string& delimiters) {
			for (int i = 0; i < delimiters.size(); ++i)
			{
				str.erase(std::remove(str.begin(), str.end(), delimiters[i]), str.end());
			}
		}

		std::string Trim(const std::string& str) {
			size_t first = str.find_first_not_of(' ');
			if (first == std::string::npos)
			{
				return str;
			}
			size_t last = str.find_last_not_of(' ');
			return str.substr(first, (last - first + 1));
		}

		void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
			if (from.empty())
				return;
			size_t start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			}
		}

		std::string ToString(glm::vec3 vec, int decimals) {
			int dec = (decimals > 0 ? decimals : 0);
			std::stringstream ss;
			ss << std::fixed << std::setprecision(dec) << '<' << vec.x << ',' << vec.y << ',' << vec.z << '>';
			return ss.str();
		}

		std::string ToString(float val, int decimals) {
			int dec = (decimals > 0 ? decimals : 0);
			std::stringstream ss;
			ss << std::fixed << std::setprecision(dec) << val;
			return ss.str();
		}
	}
}
