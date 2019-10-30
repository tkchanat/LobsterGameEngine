#pragma once

namespace Lobster
{

	class VertexBuffer;
	class IndexBuffer;
	class VertexLayout;

	enum class PrimitiveType : uint
	{
		POINTS = GL_POINTS,
		LINES = GL_LINES,
		TRIANGLES = GL_TRIANGLES
	};

	//	Container for storing all VertexBuffer and IndexBuffer pairs in a mesh. All pairs shares the same VertexLayout.
	class VertexArray
	{
	protected:
		int m_bufferCount;	//	Number of buffer pair (vertex and index buffer) in this vertex array object
		PrimitiveType m_primitive;
		uint* m_ids;	//	OpenGL Handles for all vertex array objects
		VertexLayout* m_vertexLayout;
		std::vector<VertexBuffer*> m_vertexBuffers;
		std::vector<IndexBuffer*> m_indexBuffers;
	public:
		VertexArray(VertexLayout* layout, std::vector<VertexBuffer*> vertexBuffers, std::vector<IndexBuffer*> indexBuffers, PrimitiveType primitive);
		~VertexArray();
		void Draw();
	};

}
