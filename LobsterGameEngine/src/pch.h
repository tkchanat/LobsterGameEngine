#pragma once
//	This is the precompiled header file (pch.h).
//	If you aren't familiar with PCHs, I suggest you to check out: https://www.youtube.com/watch?v=eSI4wctZUto
typedef unsigned int uint;
typedef unsigned char byte;

//  Standard includes
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <regex>
#include <string>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <queue>
#include <utility>
#include <vector>

//  OpenGL includes
#ifdef LOBSTER_PLATFORM_WIN
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <glad/glad.h>
#endif
#ifdef LOBSTER_PLATFORM_MAC
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// lua script
extern "C" {
#include <lua/include/lua.h>
#include <lua/include/lauxlib.h>
#include <lua/include/lualib.h>
}
#ifdef _WIN32
#pragma comment(lib, "liblua53.a")
#endif
#include <LuaBridge/LuaBridge.h>

//  Custom includes
#include "events/EventCollection.h"
#include "events/EventDispatcher.h"
#include "imgui/ImGuiConsole.h"
#include "system/FileSystem.h"
#include "system/Timer.h"
#include "system/ThreadPool.h"
#include "system/Profiler.h"
#include "utils/StringOps.h"

//  Custom Defines
#define M_PI 3.14159265358979323846
#define MAX_TEXTURE_UNIT 16
#define MAX_UNIFORM_BUFFER 16
#define MAX_BONES 64
#define MAX_BONE_INFLUENCE 4
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 64
#define MAX_DIRECTIONAL_SHADOW 2
#define MAX_POINT_SHADOW 3
#define MAX_PARTICLES 1024

#define PATH_AUDIO "audio"
#define PATH_MESHES "meshes"
#define PATH_MATERIALS "materials"
#define PATH_SHADERS "shaders"
#define PATH_TEXTURES "textures"
#define PATH_SPRITES "sprites"
#define PATH_FONT "textures/font"
#define PATH_SCRIPTS "scripts"
#define PATH_ANIMATIONS "animations"
#define PATH_SCENES "scenes"

#define BIT(x) 1 << x

// Warning suppression (not recommended)
#pragma warning(disable:4996)

namespace glm
{
    template<typename Archive> void serialize(Archive& archive, glm::vec2& v2)
    {
        archive(v2.x, v2.y);
    }
    template<typename Archive> void serialize(Archive& archive, glm::vec3& v3)
    {
        archive(v3.x, v3.y, v3.z);
    }
    template<typename Archive> void serialize(Archive& archive, glm::vec4& v4)
    {
        archive(v4.x, v4.y, v4.z, v4.w);
    }
    template<typename Archive> void serialize(Archive& archive, glm::quat& quat)
    {
        archive(quat.w, quat.x, quat.y, quat.z);
    }
}
