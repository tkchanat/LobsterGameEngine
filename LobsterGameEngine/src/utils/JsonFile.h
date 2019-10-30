#pragma once
#include "system/FileSystem.h"
#include "utils/StringOps.h"

namespace Lobster
{

	// Wrapper class for reading and writing json files.
	// Will store json files, and handle exceptions properly.
	class JsonFile {
	private:
		nlohmann::json m_json;
		bool b_fileExist = false;
		std::string m_path;
	public:
		JsonFile();
		JsonFile(const char* path);
		JsonFile(const nlohmann::json& other);
		~JsonFile();
		JsonFile getJsonValue(const char* attribute, std::string defaultVal);
		inline bool getFileExist() const { return b_fileExist; }
		// getters
		template <typename T>
		T getValue(const char* attribute, T defaultVal);
		std::string getValue(const char* attribute, const char * defaultVal);
		// setters
		template <typename T>
		void setValue(const char* attribute, T value);
	private:
		bool saveJson();
		std::string formatAttribute(const char * attribute);
	};


	// Template function definition
	template<typename T>
	inline T JsonFile::getValue(const char * attribute, T defaultVal)
	{
		std::string formattedAttribute = formatAttribute(attribute);
		T value;

		try {
			// Test if value at json pointer is present
			m_json.at(nlohmann::json::json_pointer(formattedAttribute));
			value = m_json.value(nlohmann::json::json_pointer(formattedAttribute), defaultVal);
		}
		catch (std::exception e) {
			m_json[nlohmann::json::json_pointer(formattedAttribute)] = defaultVal;
			saveJson();
			return defaultVal;
		}
		return value;
	}

	template<typename T>
	inline void JsonFile::setValue(const char * attribute, T value)
	{
		nlohmann::json * layerJson = &m_json;
		std::vector<std::string> layers = StringOps::split(attribute, '.');
		// Store the iterate progress to i
		int i = 0;
		try {
			for (; i < layers.size() - 1; i++) {
				layerJson = &(*layerJson)[layers[i]];
			}

			(*layerJson)[layers[i]] = value;
		}
		catch (std::exception e) {
		}
		saveJson();
	}

}
