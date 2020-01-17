#pragma once

namespace Lobster
{
    
    class VertexArray;

	enum class PrimitiveShape : uint {
		CUBE, SPHERE, PLANE
	};
    
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
