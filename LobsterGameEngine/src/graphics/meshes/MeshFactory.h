#pragma once

namespace Lobster
{
    
    class VertexArray;
    
    class MeshFactory
    {
    public:
        static VertexArray* Cube();
		static VertexArray* Plane();
		static VertexArray* Sphere(float radius, int du, int dv);
		static VertexArray* Grid(int row, int col);
		static VertexArray* Sprite();
    };
    
}
