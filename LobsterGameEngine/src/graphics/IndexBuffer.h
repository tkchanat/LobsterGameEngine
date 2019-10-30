#pragma once

namespace Lobster
{
    
	//	This class is for storing index data.
	//	This class is equivalent to OpenGL's element buffer object, and DirectX's index buffer.
	//	Learn more: https://learnopengl.com/Getting-started/Hello-Triangle
    class IndexBuffer
    {
    private:
        uint m_id;
        uint m_count;
    public:
        IndexBuffer();
        ~IndexBuffer();
        void Bind() const;
        void Unbind() const;
        void SetData(uint* data, uint count);
        inline uint GetCount() const { return m_count; }
        inline uint GetID() const { return m_id; }
    };
    
}
