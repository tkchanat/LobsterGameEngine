#include "pch.h"
#include "JsonFile.h"
#include "system/FileSystem.h"
#include "utils/StringOps.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <typeinfo>

namespace Lobster
{

	// Default constructor
	JsonFile::JsonFile()
	{
	}

	// Initialize from path
	JsonFile::JsonFile(const char* path) : 
		m_path(FileSystem::Path(path)) 
	{
		// Read from the given path
		std::string jsonPath = FileSystem::ReadText(FileSystem::Path(path).c_str());

		// Try to parse JSON, and set to empty json upon failure
		try {
			m_json = nlohmann::json::parse(jsonPath);
			b_fileExist = true;
		}
		catch (std::exception e) {
			m_json = {};
		}
	}

	// Copy constructor
	JsonFile::JsonFile(const nlohmann::json & other)
	{
		// Try to parse JSON, and set to empty json upon failure
		try {
			m_json = nlohmann::json::parse(other.dump());
			b_fileExist = true;
		}
		catch (std::exception e) {
			m_json = {};
		}
	}

	JsonFile::~JsonFile()
	{
		//saveJson();
	}

	// Get JSON Value from nested JSON structure.
	JsonFile JsonFile::getJsonValue(const char* attribute, std::string defaultVal) {
		std::vector<std::string> layers = StringOps::split(attribute, '.');
		std::string formattedAttribute;
		for (std::string& layer : layers) {
			formattedAttribute += '/' + layer;
		}

		try {
			return JsonFile(m_json.at(nlohmann::json::json_pointer(formattedAttribute)));
		}
		catch (std::exception e) {
			m_json[nlohmann::json::json_pointer(formattedAttribute)] = nlohmann::json::parse(defaultVal);
			return JsonFile(nlohmann::json::parse(defaultVal));
		}
	}

	// Try to get a string value - as default value will be of const char* type, we will need to cast it to string.
	std::string JsonFile::getValue(const char* attribute, const char * defaultVal) {
		return getValue<std::string>(attribute, std::string(defaultVal));
	}

	// Save the json file. As there's no need of calling save outside this class, this function is set private.
	bool JsonFile::saveJson() 
	{
		std::ofstream file(FileSystem::Path(m_path));
		file << m_json.dump(4);
		file.close();
		b_fileExist = true;

		return true;
	}

	// Format dot-separated attributes into slash-separated form.
	std::string JsonFile::formatAttribute(const char * attribute)
	{
		std::vector<std::string> layers = StringOps::split(attribute, '.');
		std::string formattedAttribute;
		for (std::string& layer : layers) {
			formattedAttribute += '/' + layer;
		}
		return formattedAttribute;
	}

}
