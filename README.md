# Lobster Engine <img src="LobsterGameEngine/lobster.png" width=32>
Lobster Engine is yet another OpenGL game engine written in C++. :video_game: :computer:

## Current Progress
![](doc/Screenshot.png)
Join and follow our roadmap at [<img src="https://d2k1ftgv7pobq7.cloudfront.net/meta/u/res/images/brand-assets/Logos/0099ec3754bf473d2bbf317204ab6fea/trello-logo-blue.png" width="80" alt="Trello">](https://trello.com/invite/b/CnX7N13z/665a3ecc44dd00868f37f27c2e5ee895/ssf)

## Engine Architecture
For details, please refer to our [Wiki Page](https://github.com/tkchanat/StealStepFYP/wiki/Engine-Architecture).

## Installation Guide
```diff
- To ensure it builds successfully, run the setup file everytime you perform a git pull.
```
### [<img src="https://cdn1.iconfinder.com/data/icons/flat-and-simple-part-1/128/microsoft-512.png" width="30">]() Windows:
1. Make sure you have [Visual Studio 2017+](https://visualstudio.microsoft.com) version to build.
2. Execute `setup.bat` and open project solution `LobsterGameEngine.sln`.
### [<img src="https://www.freeiconspng.com/uploads/apple-mac-icon-5.png" width="30">]() MacOS:
1. Make sure you have [Xcode11+](https://developer.apple.com/xcode/) to build, and [MacOS Catalina+](https://www.apple.com/hk/en/macos/catalina/) to run.
2. Execute `setup.command` and open project workspace `LobsterGameEngine.xcodeproj`.

## Third-party Dependencies
* [cereal](https://github.com/USCiLab/cereal) - A C++11 library for serialization
* [freetype](https://www.freetype.org/) - FreeType is a freely available software library to render fonts.
* [glfw](https://github.com/glfw/glfw) - A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input
* [glm](https://github.com/g-truc/glm.git) - OpenGL Mathematics
* [glad](https://github.com/Dav1dde/glad) - Multi-Language Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specs.
* [imgui](https://github.com/ocornut/imgui) - Dear ImGui: Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies
* [imguizmo](https://github.com/CedricGuillemet/ImGuizmo) - Immediate mode 3D gizmo for scene editing and other controls based on Dear Imgui
* [spdlog](https://github.com/gabime/spdlog) - Fast C++ logging library.
* [assimp](https://github.com/assimp/assimp) - **[Binary distribution]** Official Open Asset Import Library Repository. Loads 40+ 3D file formats into one unified and clean data structure.
* [lua5.3](https://www.lua.org/download.html) - **[Binary distribution]** Lua is a powerful and fast programming language that is easy to learn and use and to embed into your application.
* [openal](https://www.openal.org/) - [**Binary distribution]** OpenAL is a cross-platform 3D audio API appropriate for use with gaming applications and many other types of audio applications.