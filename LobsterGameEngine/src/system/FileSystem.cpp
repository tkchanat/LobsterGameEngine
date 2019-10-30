#include "pch.h"
#include "FileSystem.h"

namespace Lobster {

	FileSystem* FileSystem::m_instance = nullptr;
	std::string FileSystem::m_workingDir;
	std::map<std::string, std::vector<std::string>> FileSystem::m_directory;

	FileSystem::FileSystem() {
		if (m_instance)
			throw std::runtime_error("File system already created. Do you really need two file systems?");
		m_instance = this;
	}

	std::string FileSystem::Path(std::string path) {
		// sometimes it yields a/b\c/d, nevermind it is just fine
		// remove the two slashes suffix
		path = (path.substr(0, 1).compare("/") == 0 ? path.substr(1, path.size() - 1) : path);
		return Join(m_workingDir, path);
	}

	// To join the two string with a slash regardless of OS
	std::string FileSystem::Join(const std::string& path, const std::string& path2) {
		return path.empty()? path2 : path2.empty()? path : path + "/" + path2;
	}

	// Used in DEBUG
	bool FileSystem::assignWorkingDirectory(std::string dir)
	{
		m_workingDir = dir;
		return false;
	}

	// Used in RELEASE
	bool FileSystem::createWorkingDirectory(std::string dir)
	{
		bool err = fs::create_directories(dir);
		if (!err) m_workingDir = Path(dir);
		return err;
	}

	// Update the tracking directory by scanning all files inside
	void FileSystem::update() {
		m_directory.clear();
		for (const auto& entry : fs::directory_iterator(m_workingDir)) {
			if (entry.is_directory()) {
				fs::path path = entry.path();				
				std::vector<std::string> category;
				// going into the folder and scan all files inside
				for (const auto& inner : fs::directory_iterator(entry)) {
					category.push_back(inner.path().string());
				}
				std::pair<std::string, std::vector<std::string>> pair(path.stem().string(), category);
				m_directory.insert(pair);
			}
		}
	}

	// Add the input file into resources, clone it and return the full path of the clone
	// If the file is already included, so far not yet handled (duplication occurs)
	// Now assuming the folder with name "type" always exists
	std::string FileSystem::addResource(std::string path, std::string type) {	
		fs::path source = path;
		fs::path target = m_workingDir;
		target = Join(target.string(), (type.size() == 0 ? source.filename().string() : Join(type, source.filename().string())));
		LOG(target.string() + " loaded");
		if (!fs::exists(target))
			fs::copy_file(source, target);
		// [Exception Unhandled] push into the vector of resources of the corresponding type
		m_directory[type].push_back(target.string());
		return target.string();
	}

	// Untrack the given file [not implemented] and delete from disk if required
	// If it does not exist, do nothing and return false
	bool FileSystem::removeResource(std::string path, std::string type, bool del) {
		std::remove(m_directory[type].begin(), m_directory[type].end(), path);
		std::string full = Path(path);
		if (del)
			return fs::remove(full);
		return true;
	}

	// Read plain text file and return its content. 
	// If the file was not found, empty string will be returned.
	std::string FileSystem::ReadText(const char * path)
	{
		std::string output = "";
		std::ifstream file(path);
		if (file.is_open())
		{
			output = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
			file.close();
		}
		else
		{
			WARN("Text file {} does not exist, please check if input is valid.", path);
		}
		return output;
	}
}
