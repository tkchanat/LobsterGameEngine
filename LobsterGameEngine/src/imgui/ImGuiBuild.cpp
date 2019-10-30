#include "pch.h"
#include <GLFW/glfw3.h>

#ifdef LOBSTER_PLATFORM_WIN
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#endif
#ifdef LOBSTER_PLATFORM_MAC
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <string>    //just include a useless header
#endif

#include "examples/imgui_impl_opengl3.cpp"
#include "examples/imgui_impl_glfw.cpp"
