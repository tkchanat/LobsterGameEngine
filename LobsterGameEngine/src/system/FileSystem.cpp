#include "pch.h"
#include "FileSystem.h"

#ifdef LOBSTER_PLATFORM_WIN
#include <Commdlg.h>
#include <shlobj_core.h>
#endif

namespace Lobster {

	FileSystem* FileSystem::m_instance = nullptr;
	std::string FileSystem::m_workingDir;
	std::string FileSystem::m_executableDir;
	std::map<std::string, std::vector<std::string>> FileSystem::m_directory;

	FileSystem::FileSystem() {
		if (m_instance)
			throw std::runtime_error("File system already created. Do you really need two file systems?");
		m_instance = this;
		m_executableDir = fs::current_path().string();
	}

	// Convert the provided path into relative path, with the following format:
	// #Absolute path:		Copy into project resources and return the relative path inside resources folder
	// #<folder>/<file>:	Append working directory before the file 
	// #Relative path:		NOT SUPPORTED! Never provide this path to the function though this function tries its best to ignore.
	// Note: Path can take both <res-folder>/<file> or absolute path, but not other format
	std::string FileSystem::Path(std::string path, int flags) {
		// if path is absolute, add it into res/ and use the relative one
		fs::path p(path);
		if (p.is_absolute()) {
			// not allow to copy but absolute => just do nothing and return original path
			if (flags & Flag_SuppressCopying) return path;
			// make use of current_path to move relatively
			fs::path tempCurrentPath = fs::current_path();
			fs::current_path(m_workingDir);
			fs::path pathRelativeToWorkingDir = fs::relative(p);
			fs::current_path(tempCurrentPath); // restore current path
			// if path is already inside res/, just change the path into relative without adding
			if (pathRelativeToWorkingDir.empty() || pathRelativeToWorkingDir.string()[0] == '.') {
				std::string relative = m_instance->addResourceIfNecessary(path);
				if (!relative.empty())
					return relative;
				else {
					char msg[512];
					sprintf(msg, "FileSystem::Path() does not support conversion of path %s", path.c_str());
					throw std::exception(msg);
				}
			}
			else {
				std::string return_path = pathRelativeToWorkingDir.string();
				StringOps::ReplaceAll(return_path, "\\", "/");
				return return_path;
			}
		}
		else {
			// if path is relative starting with ../, ignore and return the original path
			if (path[0] == '.') 
				return path;
			// remove the two slashes suffix
			if (path.substr(0, 1) == "/") path.substr(1, path.size() - 1);
			std::string return_path = Join(m_workingDir, path);
			StringOps::ReplaceAll(return_path, "\\", "/");
			return return_path;
		}
	}

	std::string FileSystem::PathUnderRes(std::string path)
	{
		size_t pos = path.find(m_instance->m_workingDir);
		if (pos != std::string::npos) {
			return path.substr(pos + m_instance->m_workingDir.size() + 1);
		}
		return path;
	}

	std::string FileSystem::RelativeToAbsolute(std::string path)
	{
		return fs::canonical(fs::path(path)).string();
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
		target = Join(target.string(), (type.empty() ? source.filename().string() : Join(type, source.filename().string())));
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
			subfolder = PATH_MESHES;
		}
		else if (p.extension() == ".mat") {
			subfolder = PATH_MATERIALS;
		}
		else if (p.extension() == ".glsl") {
			subfolder = PATH_SHADERS;
		}
		else if (p.extension() == ".png" || p.extension() == ".jpg") {
			subfolder = PATH_TEXTURES;
		}
		else if (p.extension() == ".wav") {
			subfolder = PATH_AUDIO;
		}
		else if (p.extension() == ".anim") {
			subfolder = PATH_ANIMATIONS;
		}
		else if (p.extension() == ".lua") {
			subfolder = PATH_SCRIPTS;
		}
		else if (p.extension() == ".lobster") {
			subfolder = PATH_SCENES;
		}
		else {
			throw std::runtime_error("Please add new extension");
			return "";
		}
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
		wchar_t lpBuffer[MAX_FILE_BUFFER_SIZE];

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
		path = buffer;
		StringOps::ReplaceAll(path, "\\", "/");
		fs::current_path(executablePath);
		return path;
	}

	std::string FileSystem::OpenDirectoryDialog()
	{
		char buffer[MAX_FILE_BUFFER_SIZE];
		std::string path;

#ifdef LOBSTER_PLATFORM_MAC
		throw std::runtime_error("We don't support mac for now!");
#elif defined LOBSTER_PLATFORM_WIN

		BROWSEINFO browseinfo;
		wchar_t pszDisplayName[MAX_FILE_BUFFER_SIZE];
		memset(&browseinfo, 0, sizeof(BROWSEINFO));
		browseinfo.hwndOwner = NULL;
		browseinfo.pidlRoot = NULL;
		browseinfo.pszDisplayName = pszDisplayName;
		browseinfo.lpszTitle = L"Select Folder";
		browseinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		browseinfo.lpfn = NULL;
		browseinfo.lParam = NULL;
		browseinfo.iImage = 0;

		LPITEMIDLIST pidl = NULL;
		if ((pidl = SHBrowseForFolder(&browseinfo)) != NULL) {
			wchar_t buf[MAX_FILE_BUFFER_SIZE];
			if (SHGetPathFromIDList(pidl, buf)) {
				int pos = 0;
				while (buf[pos] != '\0') {
					buffer[pos] = (char)buf[pos];
					pos++;
				}
				buffer[pos] = '\0';
			}
		}
		else {
			buffer[0] = '\0';
		}
#else
		throw std::runtime_error("We don't support linux for now!");
#endif

		if (buffer[0] == '\0') return "";
		path = buffer;
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
		return fs::last_write_time(fs::path(path));
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
		else {
			WARN("Unable to save file at {}.", path);
		}				
	}
}
