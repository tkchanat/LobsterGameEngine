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
#include <nlohmann/json.hpp>

//  Custom includes
#include "events/EventCollection.h"
#include "events/EventDispatcher.h"
#include "imgui/ImGuiConsole.h"
#include "imgui/ImGuiFileBrowser.h"
#include "system/FileSystem.h"
#include "system/Timer.h"
#include "system/ThreadPool.h"
#include "system/Profiler.h"
#include "utils/StringOps.h"

//  Custom Defines
#define MAX_TEXTURE_UNIT 16
#define MAX_UNIFORM_BUFFER 16

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
        archive(quat.x, quat.y, quat.z, quat.w);
    }
}
