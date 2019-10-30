#pragma once

namespace Lobster {
    
	//	This class is for storing vertex data.
	//	This class is equivalent to OpenGL's and DirectX's vertex buffer object.
	//	Learn more: https://learnopengl.com/Getting-started/Hello-Triangle
    enum DrawMode : uint {
        STATIC_DRAW = GL_STATIC_DRAW,
        DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
        STREAM_DRAW = GL_STREAM_DRAW
    };

    class VertexBuffer
    {
    private:
        uint m_id;
        DrawMode m_mode;
    public:
        VertexBuffer(DrawMode mode = STATIC_DRAW);
        ~VertexBuffer();
        void Bind() const;
        void Unbind() const;
        void SetData(const void* data, uint size);
        inline uint GetID() const { return m_id; }
    };
    
}
