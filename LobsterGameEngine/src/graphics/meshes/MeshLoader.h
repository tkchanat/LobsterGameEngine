#pragma once

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
        static std::pair<glm::vec3, glm::vec3> Load(const char* path, std::vector<VertexArray*>& vertexArrays, std::vector<Material*>& materialArrays);
    };
    
}
