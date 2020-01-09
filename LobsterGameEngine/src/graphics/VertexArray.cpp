#include "pch.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexLayout.h"
#include "IndexBuffer.h"

namespace Lobster
{

	VertexArray::VertexArray(VertexLayout* layout, std::vector<VertexBuffer*> vertexBuffers, std::vector<IndexBuffer*> indexBuffers, PrimitiveType primitive) :
		m_bufferCount(0),
		m_primitive(primitive),
		m_ids(nullptr),
		m_vertexLayout(layout),
		m_vertexBuffers(vertexBuffers),
		m_indexBuffers(indexBuffers)
	{
		//  Initialize buffer array
		m_bufferCount = m_indexBuffers.size();
		m_ids = new uint[m_bufferCount];
		memset(m_ids, 0, sizeof(uint) * m_bufferCount);

		//  Initialize arrays in GPU
		glGenVertexArrays(m_bufferCount, m_ids);

		//  Bind all VBOs and EBOs into our VAOs,
		//  so that we don't have to bind those buffers every time.
		//  Assuming the contents in the vertex arrays don't change.
		for (int i = 0; i < m_bufferCount; ++i)
		{
			glBindVertexArray(m_ids[i]);
			m_vertexBuffers[i]->Bind();
			layout->Bind();
			m_indexBuffers[i]->Bind();
			glBindVertexArray(0);
		}
	}

	VertexArray::~VertexArray()
	{
		//  Release memory in GPU
		glDeleteVertexArrays(m_bufferCount, m_ids);

		//  Release memory in CPU
		for (auto vb : m_vertexBuffers) if (vb) delete vb;
		for (auto ib : m_indexBuffers) if (ib) delete ib;
		if (m_vertexLayout) delete m_vertexLayout;
		if (m_ids)	delete[] m_ids;
		m_ids = nullptr;
	}

	void VertexArray::Draw()
	{
		//  Bind each vertex array and draw according to the index count
		for (int i = 0; i < m_bufferCount; ++i)
		{
			glBindVertexArray(m_ids[i]);
			glDrawElements((GLenum)m_primitive, m_indexBuffers[i]->GetCount(), GL_UNSIGNED_INT, NULL);
		}
	}

}
