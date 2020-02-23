#pragma once

#include <filesystem> // c++17 warning

namespace Lobster
{
	namespace fs = std::filesystem;

	//	This class is for handling all file system work. To be implemented.
	//	e.g. Converting relative and full paths, locating resources, defining workspace directory, mounting a volume, etc.    
	class FileSystem
    {
	public:
		// Flags for FileSystem::Path()
		enum PathFlags {
			Flag_None = 0,
			Flag_SuppressCopying = 1 << 0, // not to copy the file into resources folder
		};
    private:
        static FileSystem* m_instance;
		// The root path storing everything, should be dependent in DEBUG/RELEASE
		static std::string m_workingDir;
		// A dicationary that map Resource type into a vector of resources files
		static std::map<std::string, std::vector<std::string>> m_directory;

		// TODO add textures, audio, images, script, etc...	
    public:
		FileSystem();
		inline static FileSystem* GetInstance() { return m_instance; }
		inline static std::string GetCurrentWorkingDirectory() { return m_instance->m_workingDir; }
		inline static std::map<std::string, std::vector<std::string>> GetDirectoryStructure() { return m_instance->m_directory; }				
		inline static bool Exist(const std::string& path) { return std::filesystem::exists(path); }
		static std::string Join(const std::string& path, const std::string& path2);
		static std::string Path(std::string path, int flags = PathFlags::Flag_None);
		static std::string ReadText(const char* path);
		static std::string OpenFileDialog();
		static std::string SaveFileDialog(const char* defaultPath = "");
		static std::filesystem::file_time_type LastModified(const char* path);
		static std::stringstream ReadStringStream(const char* path, bool binary = false);
		static void WriteStringStream(const char* path, const std::stringstream& ss, bool binary = false);
		
		bool assignWorkingDirectory(std::string dir); // for DEBUG
		bool createWorkingDirectory(std::string dir); // for RELEASE
		std::string addResourceIfNecessary(std::string path);
		std::string addResource(std::string path, std::string type = "meshes");
		bool removeResource(std::string path, std::string type = "meshes", bool del = true);		
		void update();
    };  
}
