#include "pch.h"
#include "VertexLayout.h"

namespace Lobster {
    
    VertexLayout::VertexLayout() :
        m_stride(0)
    {
    }
    
    VertexLayout::~VertexLayout()
    {
    }
    
    void VertexLayout::Bind() const
    {
        for (uint i = 0; i < m_layout.size(); ++i)
        {
            const LayoutElement& element = m_layout[i];
			if(element.type == GL_UNSIGNED_INT || element.type == GL_INT)
				glVertexAttribIPointer(i, element.count, element.type, m_stride, (void*)element.offset);
			else
				glVertexAttribPointer(i, element.count, element.type, element.normalized, m_stride, (void*)element.offset);
            glEnableVertexAttribArray(i);
        }
    }
    
    void VertexLayout::Unbind() const
    {
        
    }
    
    
    //=====================================
    //  Explicit Template Specializations
    //=====================================
	template<typename T>
	void Add(const char* name, uint count, uint offset, bool normalized) 
	{
		throw std::runtime_error("This type is not supported yet, please extend the template specialization below");
	}

    template<>
    void VertexLayout::Add<float>(const char* name, uint count)
    {
        m_layout.emplace_back(name, sizeof(float), GL_FLOAT, false, count, m_stride);
        m_stride += sizeof(float) * count;
    }

	template<>
	void VertexLayout::Add<int>(const char* name, uint count)
	{
		m_layout.emplace_back(name, sizeof(int), GL_INT, false, count, m_stride);
		m_stride += sizeof(int) * count;
	}
    
    template<>
    void VertexLayout::Add<uint>(const char* name, uint count)
    {
        m_layout.emplace_back(name, sizeof(uint), GL_UNSIGNED_INT, false, count, m_stride);
        m_stride += sizeof(uint) * count;
    }
    
}
