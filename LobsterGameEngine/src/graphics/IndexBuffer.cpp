#include "pch.h"
#include "IndexBuffer.h"

namespace Lobster
{
    
    IndexBuffer::IndexBuffer() :
        m_id(0),
        m_count(0)
    {
        glGenBuffers(1, &m_id);
    }
    
    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }
    
    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    }
    
    void IndexBuffer::Unbind() const
    {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    void IndexBuffer::SetData(uint *data, uint count)
    {
        m_count = count;
		Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(uint), data, GL_STATIC_DRAW);
    }
    
}
