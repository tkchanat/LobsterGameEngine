#include "pch.h"
#include "FileSystem.h"

#ifdef LOBSTER_PLATFORM_WIN
#include <Commdlg.h>
#endif

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
		// if path is absolute, add it into res/ and use the relative one		
		// TODO delete this if not required
		fs::path p(path);
		if (p.is_absolute()) {
			std::string relative = m_instance->addResourceIfNecessary(path);
			if (!relative.empty())
				return relative;
		}
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

	// Same as addResource(std::string path, std::string type),
// except letting the system determine the type itself
// If no file is added, empty string will be returned
	std::string FileSystem::addResourceIfNecessary(std::string path) {
		// TODO check any object with the same name
		fs::path p(path);
		std::string subfolder;
		if (p.extension() == ".obj" || p.extension() == ".mtl") {
			subfolder = "meshes";
		}
		else if (p.extension() == ".mat") {
			subfolder = "materials";
		}
		else if (p.extension() == ".glsl") {
			subfolder = "shaders";
		}
		else if (p.extension() == ".png") {
			subfolder = "textures";
		}
		else return "";
		addResource(p.string(), subfolder);
		return Path(Join(subfolder, p.filename().string()));
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

	std::string FileSystem::OpenFileDialog()
	{
		const int FILE_DIALOG_MAX_BUFFER = 1024;
		char buffer[FILE_DIALOG_MAX_BUFFER];
		std::string path;

		fs::path executablePath = fs::current_path();

#ifdef LOBSTER_PLATFORM_MAC
		// For apple use applescript hack
		FILE * output = popen(
			"osascript -e \""
			"   tell application \\\"System Events\\\"\n"
			"           activate\n"
			"           set existing_file to choose file\n"
			"   end tell\n"
			"   set existing_file_path to (POSIX path of (existing_file))\n"
			"\" 2>/dev/null | tr -d '\n' ", "r");
		while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
		{
		}
#elif defined LOBSTER_PLATFORM_WIN

		// Use native windows file dialog box
		OPENFILENAME ofn;       // common dialog box structure
		wchar_t lpBuffer[FILE_DIALOG_MAX_BUFFER];

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = lpBuffer;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 260;
		ofn.lpstrFilter = L"*.*\0";//off\0*.off\0obj\0*.obj\0mp\0*.mp\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Display the Open dialog box. 
		int pos = 0;
		if (GetOpenFileName(&ofn) == TRUE)
		{
			while (ofn.lpstrFile[pos] != '\0')
			{
				buffer[pos] = (char)ofn.lpstrFile[pos];
				pos++;
			}
		}
		else return ""; // dialog closed
		buffer[pos] = 0;
#else

		// For linux use zenity
		FILE * output = popen("/usr/bin/zenity --file-selection", "r");
		while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
		{
		}

		if (strlen(buffer) > 0)
		{
			buffer[strlen(buffer) - 1] = 0;
		}
		
#endif        
        path = fs::relative(fs::path(buffer), executablePath).string();
        StringOps::ReplaceAll(path, "\\", "/");
        fs::current_path(executablePath);
		return path;
	}

	std::string FileSystem::SaveFileDialog(const char* defaultPath) {
		const int FILE_DIALOG_MAX_BUFFER = 1024;
		char buffer[FILE_DIALOG_MAX_BUFFER];
		std::string path;

		fs::path executablePath = fs::current_path();

#ifdef LOBSTER_PLATFORM_MAC
		// For apple use applescript hack
		FILE * output = popen(
			"osascript -e \""
			"   tell application \\\"System Events\\\"\n"
			"           activate\n"
			"           set existing_file to choose file\n"
			"   end tell\n"
			"   set existing_file_path to (POSIX path of (existing_file))\n"
			"\" 2>/dev/null | tr -d '\n' ", "r");
		while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
		{
		}
#elif defined LOBSTER_PLATFORM_WIN

		// Use native windows file dialog box
		OPENFILENAME ofn;       // common dialog box structure
		wchar_t lpBuffer[FILE_DIALOG_MAX_BUFFER];

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = lpBuffer;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 260;
		ofn.lpstrFilter = L"Lobster Scene (*.lobster)\0*.lobster\0All Files (*.*)\0*.*\0";
		ofn.lpstrDefExt = (LPCWSTR)L"lobster";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;		
		ofn.lpstrInitialDir = (LPCWSTR)defaultPath;
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;	

		// Display the Open dialog box. 
		int pos = 0;
		if (GetSaveFileName(&ofn))
		{
			while (ofn.lpstrFile[pos] != '\0')
			{
				buffer[pos] = (char)ofn.lpstrFile[pos];
				pos++;
			}
		}
		else return ""; // dialog closed
		buffer[pos] = 0;
#else

		// For linux use zenity
		FILE * output = popen("/usr/bin/zenity --file-selection", "w");
		while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
		{
		}

		if (strlen(buffer) > 0)
		{
			buffer[strlen(buffer) - 1] = 0;
		}

#endif
		path = buffer;
		StringOps::ReplaceAll(path, "\\", "/");
		fs::current_path(executablePath);
		return path;
	}

	std::filesystem::file_time_type FileSystem::LastModified(const char * path)
	{
		return fs::last_write_time(fs::path(Path(path)));
	}

	std::stringstream FileSystem::ReadStringStream(const char * path, bool binary)
	{
		int flags = std::ios::in;
		if (binary) flags |= std::ios::binary;
		std::ifstream inFile(path, flags);
		std::stringstream ss;
		if (inFile.is_open()) {
			ss << inFile.rdbuf();
			inFile.close();
		}
		return ss;
	}

	void FileSystem::WriteStringStream(const char * path, const std::stringstream & ss, bool binary)
	{
		int flags = std::ios::out;
		if (binary) flags |= std::ios::binary;
		std::ofstream outFile(path, flags);
		if (outFile.is_open()) {
			outFile << ss.rdbuf();
			outFile.close();
		}
	}
}
