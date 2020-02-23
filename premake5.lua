workspace "LobsterGameEngine"
	configurations { "Debug", "Release" }
	architecture "x64"
	startproject "LobsterGameEngine"

-- Third-party Dependencies
project "GLFW"
	location "LobsterGameEngine/vendor"
	kind "StaticLib"
	language "C"
	systemversion "latest"

	targetdir ("LobsterGameEngine/lib")
	objdir("LobsterGameEngine/vendor/bin-int/%{prj.name}")
	includedirs "LobsterGameEngine/vendor/glfw/include"

	files {
		"LobsterGameEngine/vendor/glfw/include/GLFW/glfw3.h",
		"LobsterGameEngine/vendor/glfw/include/GLFW/glfw3native.h",
		"LobsterGameEngine/vendor/glfw/src/glfw_config.h",
		"LobsterGameEngine/vendor/glfw/src/context.c",
		"LobsterGameEngine/vendor/glfw/src/egl_context.h",
		"LobsterGameEngine/vendor/glfw/src/egl_context.c",
		"LobsterGameEngine/vendor/glfw/src/init.c",
		"LobsterGameEngine/vendor/glfw/src/input.c",
		"LobsterGameEngine/vendor/glfw/src/monitor.c",
		"LobsterGameEngine/vendor/glfw/src/osmesa_context.h",
		"LobsterGameEngine/vendor/glfw/src/osmesa_context.c",
		"LobsterGameEngine/vendor/glfw/src/vulkan.c",
		"LobsterGameEngine/vendor/glfw/src/window.c"
	}

	filter "system:windows"
		files {
			"LobsterGameEngine/vendor/glfw/src/win32_*",
			"LobsterGameEngine/vendor/glfw/src/wgl_context.h",
			"LobsterGameEngine/vendor/glfw/src/wgl_context.c"
		}
		defines "_GLFW_WIN32"

	filter "system:macosx"
		files {
			"LobsterGameEngine/vendor/glfw/src/cocoa_*",
			"LobsterGameEngine/vendor/glfw/src/posix_thread.h",
			"LobsterGameEngine/vendor/glfw/src/posix_thread.c",
			"LobsterGameEngine/vendor/glfw/src/nsgl_context.h",
			"LobsterGameEngine/vendor/glfw/src/nsgl_context.m"
		}
		defines "_GLFW_COCOA"

project "GLAD"
	location "LobsterGameEngine/vendor"
	kind "StaticLib"
	language "C"
	systemversion "latest"

	targetdir ("LobsterGameEngine/lib")
	objdir ("LobsterGameEngine/vendor/bin-int/%{prj.name}")
	includedirs "LobsterGameEngine/vendor/glad/include"

	files {
		"LobsterGameEngine/vendor/glad/include/**.h",
		"LobsterGameEngine/vendor/glad/src/glad.c"
	}

	filter "system:macosx"
		xcodebuildsettings { ["ALWAYS_SEARCH_USER_PATHS"] = "YES" }

project "ImGui"
	location "LobsterGameEngine/vendor"
    kind "StaticLib"
    language "C++"
    systemversion "latest"
    
	targetdir ("LobsterGameEngine/lib")
    objdir ("LobsterGameEngine/vendor/bin-int/%{prj.name}")

	files {
        "LobsterGameEngine/vendor/imgui/im**.h",
        "LobsterGameEngine/vendor/imgui/im**.cpp"
	}

project "ImGuizmo"
	location "LobsterGameEngine/vendor"
    kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    systemversion "latest"
    
	targetdir ("LobsterGameEngine/lib")
    objdir ("LobsterGameEngine/vendor/bin-int/%{prj.name}")

	links { "ImGui" }
	includedirs { "%{prj.location}/imgui" }

	files {
        "LobsterGameEngine/vendor/imguizmo/Im*.h",
        "LobsterGameEngine/vendor/imguizmo/Im*.cpp"
    }

-- Template
project "LobsterTemplate"
	location "LobsterGameEngine"
	language "C++"
	cppdialect "C++17"
	dependson { "GLFW", "GLAD", "ImGui", "ImGuizmo" }
	
	defines "LOBSTER_BUILD_TEMPLATE"
	targetdir "bin/%{cfg.buildcfg}/templates"
	targetname "%{cfg.system}_template"
	objdir "bin/int/%{cfg.buildcfg}"
	
	libdirs	{
		"%{prj.location}/vendor/assimp/lib",
		"%{prj.location}/vendor/openal/libs/Win64",
		"%{prj.location}/vendor/freetype/win64",
		"%{prj.location}/vendor/lua535/lua"
	}

	files {
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/LobsterGameEngine.rc",
		"%{prj.location}/lobster.ico"
	}

	includedirs {
		"%{prj.location}/src",
		"%{prj.location}/vendor/assimp/include",
		"%{prj.location}/vendor/stb",
		"%{prj.location}/vendor/cereal/include",
		"%{prj.location}/vendor/glm",
		"%{prj.location}/vendor/glfw/include",
		"%{prj.location}/vendor/glad/include",
		"%{prj.location}/vendor/imgui",
		"%{prj.location}/vendor/imguizmo",
		"%{prj.location}/vendor/json/include",
		"%{prj.location}/vendor/spdlog/include",
		"%{prj.location}/vendor/openal/include",
		"%{prj.location}/vendor/freetype/include",
		"%{prj.location}/vendor/lua535"
	}

	filter "system:windows"
		entrypoint "mainCRTStartup"
		systemversion "latest"
		debugdir "bin/%{cfg.buildcfg}"
		defines { "LOBSTER_PLATFORM_WIN" }
		icon "lobster.ico"

		pchheader "pch.h"
		pchsource "%{prj.location}/src/pch.cpp"

		links {
			"GLFW",
			"assimp-vc140-mt",
			"ImGui",
			"ImGuizmo",
			"GLAD",
			"opengl32",
			"OpenAL32",
			"freetype"
		}

		local root_path = "../bin/%{cfg.buildcfg}/templates/"
		local res_path = "../bin/%{cfg.buildcfg}/resources/"
		optimize "Full"
		kind "WindowedApp"
		prebuildcommands {
			"{COPY} vendor/assimp/lib/ " .. root_path,
			"{COPY} vendor/lua535/lua/dll/ " .. root_path,
			"{COPY} vendor/freetype/win64/ " .. root_path
		}
		postbuildcommands {
			"{DELETE} " .. root_path .. "*.lib",
			"{DELETE} " .. root_path .. "*.dll",
			"{DELETE} " .. root_path .. "*.pdb",
			"{DELETE} " .. root_path .. "*.ilk",
			"{DELETE} " .. root_path .. "*.dylib"
		}
		
	filter "system:macosx"
		kind "ConsoleApp"
		pchheader "src/pch.h"
		pchsource "%{prj.location}/src/pch.cpp"
		xcodebuildsettings { ["ALWAYS_SEARCH_USER_PATHS"] = "YES" }

		links {
			"GLFW",
			"assimp4.1.0",
			"ImGui",
			"ImGuizmo",
			"GLAD",
			"freetype",
			"Cocoa.framework",
			"OpenAL.framework",
			"OpenGL.framework",
			"IOKit.framework",
			"CoreVideo.framework"
		}

		defines {
			"LOBSTER_PLATFORM_MAC"
		}

-- LobsterGameEngine Core Engine
project "LobsterGameEngine"
	location "LobsterGameEngine"
	language "C++"
	cppdialect "C++17"
	dependson { "GLFW", "GLAD", "ImGui", "ImGuizmo", "LobsterTemplate" }

	targetdir ("bin/%{cfg.buildcfg}")
	objdir ("bin/int/%{cfg.buildcfg}")
	libdirs	{
		"%{prj.location}/vendor/assimp/lib",
		"%{prj.location}/vendor/openal/libs/Win64",
		"%{prj.location}/vendor/freetype/win64",
		"%{prj.location}/vendor/lua535/lua"
	}

	files {
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/LobsterGameEngine.rc",
		"%{prj.location}/lobster.ico"
	}

	includedirs {
		"%{prj.location}/src",
		"%{prj.location}/vendor/assimp/include",
		"%{prj.location}/vendor/stb",
		"%{prj.location}/vendor/cereal/include",
		"%{prj.location}/vendor/glm",
		"%{prj.location}/vendor/glfw/include",
		"%{prj.location}/vendor/glad/include",
		"%{prj.location}/vendor/imgui",
		"%{prj.location}/vendor/imguizmo",
		"%{prj.location}/vendor/json/include",
		"%{prj.location}/vendor/spdlog/include",
		"%{prj.location}/vendor/openal/include",
		"%{prj.location}/vendor/freetype/include",
		"%{prj.location}/vendor/lua535"
	}

	filter "system:windows"
		entrypoint "mainCRTStartup"
		systemversion "latest"
		debugdir "bin/%{cfg.buildcfg}"
		defines { "LOBSTER_PLATFORM_WIN" }
		icon "lobster.ico"

		pchheader "pch.h"
		pchsource "%{prj.location}/src/pch.cpp"

		links {
			"GLFW",
			"assimp-vc140-mt",
			"ImGui",
			"ImGuizmo",
			"GLAD",
			"opengl32",
			"OpenAL32",
			"freetype"
		}

		local root_path = "../bin/%{cfg.buildcfg}/"
		local res_path = "../bin/%{cfg.buildcfg}/resources/"
		filter "Debug"
			optimize "Debug"
			kind "ConsoleApp"
			defines { "LOBSTER_BUILD_DEBUG", "LOBSTER_BUILD_EDITOR", "USE_SPDLOG_CONSOLE" }
			prebuildcommands {
				"{COPY} vendor/assimp/lib/ " .. root_path,
				"{COPY} vendor/lua535/lua/dll/ " .. root_path,
				"{COPY} vendor/freetype/win64/ " .. root_path
			}
		filter "Release"
			optimize "On"
			kind "WindowedApp"
			defines { "LOBSTER_BUILD_RELEASE", "LOBSTER_BUILD_EDITOR" }
			prebuildcommands {
				"{MKDIR} " .. res_path,
				"{COPY} res/ " .. res_path,
				"{COPY} vendor/assimp/lib/ " .. root_path,
				"{COPY} vendor/lua535/lua/dll/ " .. root_path,
				"{COPY} vendor/freetype/win64/ " .. root_path
			}
			postbuildcommands {
				-- "{DELETE} " .. root_path .. "*.lib",
				-- "{DELETE} " .. root_path .. "*.pdb",
				-- "{DELETE} " .. root_path .. "*.ilk",
				-- "{DELETE} " .. root_path .. "*.dylib"
			}
		
	filter "system:macosx"
		kind "ConsoleApp"
		pchheader "src/pch.h"
		pchsource "%{prj.location}/src/pch.cpp"
		xcodebuildsettings { ["ALWAYS_SEARCH_USER_PATHS"] = "YES" }

		links {
			"GLFW",
			"assimp4.1.0",
			"ImGui",
			"ImGuizmo",
			"GLAD",
			"freetype",
			"Cocoa.framework",
			"OpenAL.framework",
			"OpenGL.framework",
			"IOKit.framework",
			"CoreVideo.framework"
		}

		defines {
			"LOBSTER_PLATFORM_MAC"
		}