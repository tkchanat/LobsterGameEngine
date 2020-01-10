#pragma once
#include "components/MeshComponent.h"

namespace Lobster
{

	class Material;
    class VertexArray;
    
    //  TODO:
    //  Now this class is a wrapper class, everything are placeholder
    //  Implement this class if we don't to use library for loading meshes
    class MeshLoader
    {
    public:
        static MeshInfo Load(const char* path);
        static std::vector<AnimationInfo> LoadAnimation(const char* path);
    };
    
}
