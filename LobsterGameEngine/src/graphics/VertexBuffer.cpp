#include "pch.h"
#include "VertexBuffer.h"

namespace Lobster {
    
    VertexBuffer::VertexBuffer(DrawMode mode) :
        m_id(0),
        m_mode(mode)
    {
        glGenBuffers(1, &m_id);
    }
    
    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }
    
    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
    }
    
    void VertexBuffer::Unbind() const
    {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void VertexBuffer::SetData(const void* data, uint size)
    {
        //  TODO:
        //  Provide interface for user choose buffer type.
        //  GL_STATIC_DRAW: the data will most likely not change at all or very rarely.
        //  GL_DYNAMIC_DRAW: the data is likely to change a lot.
        //  GL_STREAM_DRAW: the data will change every time it is drawn.
		Bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, (GLenum)m_mode);
    }
    
}
