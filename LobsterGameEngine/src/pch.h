#pragma once
//	This is the precompiled header file (pch.h).
//	If you aren't familiar with PCHs, I suggest you to check out: https://www.youtube.com/watch?v=eSI4wctZUto

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
#include <nlohmann/json.hpp>

//  Custom includes
#include "typedefs.h"
#include "events/EventCollection.h"
#include "events/EventDispatcher.h"
#include "imgui/ImGuiConsole.h"
#include "imgui/ImGuiFileBrowser.h"
#include "system/FileSystem.h"
#include "system/Timer.h"
#include "system/ThreadPool.h"
#include "system/Profiler.h"
#include "utils/JsonFile.h"
#include "utils/StringOps.h"

//  Custom Defines
#define MAX_TEXTURE_UNIT 16
#define MAX_UNIFORM_BUFFER 16

// Warning suppression (not recommended)
#pragma warning(disable:4996)
