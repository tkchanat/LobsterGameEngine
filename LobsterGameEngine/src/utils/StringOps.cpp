#include "pch.h"
#include "StringOps.h"

namespace Lobster
{
	namespace StringOps
	{
		std::vector<std::string> split(std::string str, char delimiter) {
			std::vector<std::string> splitStr;
			/*std::size_t found = 0;
			while (found != std::string::npos) {
				size_t newPos = str.find(delimiter, found);
				splitStr.push_back(str.substr(found, (newPos == std::string::npos ? std::string::npos : newPos - found)));
				found = newPos + 1;
			}
			return splitStr;*/
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, delimiter)) {
				splitStr.push_back(token);
			}
			return splitStr;
		}

		std::string substr(const std::string& str, const char* from, const char* to)
		{
			size_t begin = from == nullptr ? 0 : str.find(from);
			size_t end = to == nullptr ? std::string::npos : str.find(to);
			return str.substr(begin, end);
		}
		std::vector<std::string> RegexAllOccurrence(const std::string & str, const char * expr)
		{
			std::vector<std::string> output;
			const std::regex r(expr);
			std::smatch sm;
			if (std::regex_search(str, sm, r))
			{
				for (int i = 0; i < sm.size(); ++i)
				{
					output.push_back(sm.str(i));
				}
			}
			return output;
		}
		void Erase(std::string & str, const std::string & delimiters)
		{
			for (int i = 0; i < delimiters.size(); ++i)
			{
				str.erase(std::remove(str.begin(), str.end(), delimiters[i]), str.end());
			}
		}
		std::string Trim(const std::string & str)
		{
			size_t first = str.find_first_not_of(' ');
			if (first == std::string::npos)
			{
				return str;
			}
			size_t last = str.find_last_not_of(' ');
			return str.substr(first, (last - first + 1));
		}
	}
}
