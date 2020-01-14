workspace "LobsterGameEngine"
	configurations { "Debug", "Release" }
	architecture "x64"
	startproject "LobsterGameEngine"

	configuration "Debug"
		defines { "LOBSTER_BUILD_DEBUG" }
	configuration "Release"
		defines { "LOBSTER_BUILD_RELEASE" }

-- Third-party Dependencies
-- project "Assimp"
-- 	location "LobsterGameEngine/vendor"
-- 	kind "SharedLib"
-- 	language "C++"
-- 	cppdialect "C++17"

-- 	targetdir ("LobsterGameEngine/lib")
-- 	objdir("LobsterGameEngine/vendor/bin-int/%{prj.name}")

-- 	defines {
-- 		"ASSIMP_BUILD_NO_OWN_ZLIB",
-- 		"ASSIMP_BUILD_NO_3D_IMPORTER",
-- 		"ASSIMP_BUILD_NO_EXPORT",
-- 		"ASSIMP_BUILD_NO_C4D_IMPORTER", -- Cinema4D is MSVC only and needs some weird headers to work
-- 		"OPENDDL_STATIC_LIBARY",
-- 	}
-- 	files {
-- 		"LobsterGameEngine/vendor/assimp/include/**",
-- 		"LobsterGameEngine/vendor/assimp/code/**.cpp",
-- 		"LobsterGameEngine/vendor/assimp/code/**.h",
-- 		"LobsterGameEngine/vendor/assimp/contrib/irrXML/*.cpp",
-- 		"LobsterGameEngine/vendor/assimp/contrib/irrXML/*.h",
-- 		"LobsterGameEngine/vendor/assimp/contrib/unzip/*.c",
-- 		"LobsterGameEngine/vendor/assimp/contrib/unzip/*.h",
-- 		"LobsterGameEngine/vendor/assimp/contrib/openddlparser/code/*.cpp",
-- 		"LobsterGameEngine/vendor/assimp/contrib/poly2tri/poly2tri/**.cc",
-- 		"LobsterGameEngine/vendor/assimp/contrib/clipper/*.cpp",
-- 	}
-- 	includedirs {
-- 		"LobsterGameEngine/vendor/assimp",
-- 		"LobsterGameEngine/vendor/assimp/code",
-- 		"LobsterGameEngine/vendor/assimp/contrib/irrXML",
-- 		"LobsterGameEngine/vendor/assimp/contrib/rapidjson/include",
-- 		"LobsterGameEngine/vendor/assimp/contrib/openddlparser/include",
-- 		"LobsterGameEngine/vendor/assimp/contrib/unzip",
-- 		"LobsterGameEngine/vendor/assimp/include"
-- 	}
-- 	removefiles {
-- 		"LobsterGameEngine/vendor/assimp/code/Importer/IFC/IFCReaderGen_4.*",
-- 		"LobsterGameEngine/vendor/assimp/code/Common/Version.cpp"
-- 	}

-- 	-- prebuildcommands {
-- 		-- ("{COPY} ./assimp/include/assimp/config.h.in ./assimp/include/assimp/config.h"),
-- 		-- ("{COPY} ./assimp/revision.h.in ./assimp/revision.h")
-- 	-- }

-- 	filter "system:windows"
-- 		buildoptions "/bigobj"
-- 		defines "_CRT_SECURE_NO_WARNINGS"
-- 		disablewarnings "4065"

-- 	filter "system:macosx"
-- 		xcodebuildsettings { ["ALWAYS_SEARCH_USER_PATHS"] = "YES" }

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

-- LobsterGameEngine Core Engine
project "LobsterGameEngine"
	location "LobsterGameEngine"
	language "C++"
	cppdialect "C++17"
	dependson { "GLFW", "GLAD", "ImGui", "ImGuizmo" }

	targetdir ("bin/%{cfg.buildcfg}")
	objdir ("bin/int/%{cfg.buildcfg}")
	libdirs	{
		"%{prj.location}/vendor/assimp/lib",
		"%{prj.location}/vendor/openal/libs/Win64",
		"%{prj.location}/vendor/freetype/win64"
	}

	files {
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp"
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
	}

	filter "system:windows"
		entrypoint "mainCRTStartup"
		systemversion "latest"
		kind "ConsoleApp"
		debugdir "bin/%{cfg.buildcfg}"

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
			"freetype",
		}

		prebuildcommands {
			"{COPY} vendor/assimp/lib/ ../bin/%{cfg.buildcfg}",
			"{COPY} vendor/freetype/win64/ ../bin/%{cfg.buildcfg}"
		}

		defines {
			"LOBSTER_PLATFORM_WIN"
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