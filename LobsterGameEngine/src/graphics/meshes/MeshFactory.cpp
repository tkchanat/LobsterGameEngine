#include "pch.h"
#include "MeshFactory.h"

#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexLayout.h"

namespace Lobster
{
    
    VertexArray* MeshFactory::Cube()
    {
        std::vector<VertexBuffer*> vb;
        std::vector<IndexBuffer*> ib;
        VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
        layout->Add<float>("POSITION", 3);
		layout->Add<float>("NORMAL", 3, sizeof(float) * 3);
		layout->Add<float>("TEXCOORD", 2, sizeof(float) * 6);
        
		float vertices[288] = {
			// position         // normal       // texture
			-1.0, -1.0, 1.0,    0.0, 0.0, 1.0,    0.0, 1.0,
			1.0, -1.0, 1.0,    0.0, 0.0, 1.0,    1.0, 1.0,
			-1.0, 1.0, 1.0,    0.0, 0.0, 1.0,    0.0, 0.0,
			-1.0, 1.0, 1.0,    0.0, 0.0, 1.0,    0.0, 0.0,
			1.0, -1.0, 1.0,    0.0, 0.0, 1.0,    1.0, 1.0,
			1.0, 1.0, 1.0,    0.0, 0.0, 1.0,    1.0, 0.0,
			-1.0, 1.0, 1.0,    0.0, 1.0, 0.0,    0.0, 1.0,
			1.0, 1.0, 1.0,    0.0, 1.0, 0.0,    1.0, 1.0,
			-1.0, 1.0, -1.0,    0.0, 1.0, 0.0,    0.0, 0.0,
			-1.0, 1.0, -1.0,    0.0, 1.0, 0.0,    0.0, 0.0,
			1.0, 1.0, 1.0,    0.0, 1.0, 0.0,    1.0, 1.0,
			1.0, 1.0, -1.0,    0.0, 1.0, 0.0,    1.0, 0.0,
			-1.0, 1.0, -1.0,    0.0, 0.0, -1.0,    1.0, 0.0,
			1.0, 1.0, -1.0,    0.0, 0.0, -1.0,    0.0, 0.0,
			-1.0, -1.0, -1.0,    0.0, 0.0, -1.0,    1.0, 1.0,
			-1.0, -1.0, -1.0,    0.0, 0.0, -1.0,    1.0, 1.0,
			1.0, 1.0, -1.0,    0.0, 0.0, -1.0,    0.0, 0.0,
			1.0, -1.0, -1.0,    0.0, 0.0, -1.0,    0.0, 1.0,
			-1.0, -1.0, -1.0,    0.0, -1.0, 0.0,    0.0, 1.0,
			1.0, -1.0, -1.0,    0.0, -1.0, 0.0,    1.0, 1.0,
			-1.0, -1.0, 1.0,    0.0, -1.0, 0.0,    0.0, 0.0,
			-1.0, -1.0, 1.0,    0.0, -1.0, 0.0,    0.0, 0.0,
			1.0, -1.0, -1.0,    0.0, -1.0, 0.0,    1.0, 1.0,
			1.0, -1.0, 1.0,    0.0, -1.0, 0.0,    1.0, 0.0,
			1.0, -1.0, 1.0,    1.0, 0.0, 0.0,    0.0, 1.0,
			1.0, -1.0, -1.0,    1.0, 0.0, 0.0,    1.0, 1.0,
			1.0, 1.0, 1.0,    1.0, 0.0, 0.0,    0.0, 0.0,
			1.0, 1.0, 1.0,    1.0, 0.0, 0.0,    0.0, 0.0,
			1.0, -1.0, -1.0,    1.0, 0.0, 0.0,    1.0, 1.0,
			1.0, 1.0, -1.0,    1.0, 0.0, 0.0,    1.0, 0.0,
			-1.0, -1.0, -1.0,    -1.0, 0.0, 0.0,    0.0, 1.0,
			-1.0, -1.0, 1.0,    -1.0, 0.0, 0.0,    1.0, 1.0,
			-1.0, 1.0, -1.0,    -1.0, 0.0, 0.0,    0.0, 0.0,
			-1.0, 1.0, -1.0,    -1.0, 0.0, 0.0,    0.0, 0.0,
			-1.0, -1.0, 1.0,    -1.0, 0.0, 0.0,    1.0, 1.0,
			-1.0, 1.0, 1.0,    -1.0, 0.0, 0.0,    1.0, 0.0
		};
        
		uint indices[36] = {
			0,1,2, 3,4,5,
			6,7,8, 9,10,11,
			12,13,14, 15,16,17,
			18,19,20, 21,22,23,
			24,25,26, 27,28,29,
			30,31,32, 33,34,35
        };
        
        vertexBuffer->SetData(vertices, sizeof(vertices));
        indexBuffer->SetData(indices, 36);
        
        vb.push_back(vertexBuffer);
        ib.push_back(indexBuffer);
        return new VertexArray(layout, vb, ib, PrimitiveType::TRIANGLES);
    }

	VertexArray * MeshFactory::Plane()
	{
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
		layout->Add<float>("POSITION", 2);		
		layout->Add<float>("TEXCOORD", 2, sizeof(float) * 2);

		float vertices[24] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};
		uint indices[6] = {
			0, 1, 2,
			3, 4, 5
		};

		vertexBuffer->SetData(vertices, sizeof(vertices));
		indexBuffer->SetData(indices, 6);

		vb.push_back(vertexBuffer);
		ib.push_back(indexBuffer);
		return new VertexArray(layout, vb, ib, PrimitiveType::TRIANGLES);
	}

	// u: the division in interval [0, 2*pi]; v: the division in interval [0, pi]
	VertexArray * MeshFactory::Sphere(int radius, int du, int dv)
	{
		const double pi = 3.141592;
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
		layout->Add<float>("POSITION", 3);
		layout->Add<float>("NORMAL", 3, sizeof(float) * 3);
		layout->Add<float>("TEXCOORD", 2, sizeof(float) * 6);

		float* vertices = new float[du * dv * 8];
		float* vtx = vertices;
		uint* indices = new uint[du * dv * 6];
		uint* ind = indices;

		double R = 1. / (double)(du - 1);
		double S = 1. / (double)(dv - 1);

		for (int r = 0; r < du; r++) {
			for (int s = 0; s < dv; s++) {
				float y = sin(-pi * 0.5 + pi * r * R);
				float x = cos(2.0 * pi * s * S) * sin(pi * r * R);
				float z = sin(2.0 * pi * s * S) * sin(pi * r * R);
				// vertices, normals, texture coordinates	
				*vtx++ = x * radius;
				*vtx++ = y * radius;
				*vtx++ = z * radius;
				*vtx++ = x;
				*vtx++ = y;
				*vtx++ = z;				
				*vtx++ = (double)s * S;
				*vtx++ = (double)r * R;	
				// indice
				*ind++ = r * dv + s;
				*ind++ = r * dv + (s + 1);
				*ind++ = (r + 1) * dv + (s + 1);
				*ind++ = r * dv + s;
				*ind++ = (r + 1) * dv + (s + 1);
				*ind++ = (r + 1) * dv + s;
			}
		}	

		vertexBuffer->SetData(vertices, sizeof(*vertices) * dv * du * 8);
		indexBuffer->SetData(indices, dv * du * 6);
		vb.push_back(vertexBuffer);
		ib.push_back(indexBuffer);
		// free memory
		/*
		if (vertices) delete[] vertices;
		if (indices) delete[] indices;
		vertices = nullptr;
		indices = nullptr;*/
		return new VertexArray(layout, vb, ib, PrimitiveType::TRIANGLES);
	}


	VertexArray * MeshFactory::Grid(int row, int col)
	{
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
		layout->Add<float>("in_position", 3);

		int verticesArraySize = ((row + 1) * 2 + (col + 1) * 2) * 3; // a pair of vertex on each size of the grid end, each with 8 float values.
		float* vertices = new float[verticesArraySize];
		for (int i = 0; i < row + 1; ++i)
		{ 
			// vertex position (-z)
			vertices[i * 2 * 3 + 0] = row / 2.0f - row + i;
			vertices[i * 2 * 3 + 1] = 0.0f;
			vertices[i * 2 * 3 + 2] = col / 2.0f - col;
			// vertex position (+z)
			vertices[i * 2 * 3 + 3] = row / 2.0f - row + i;
			vertices[i * 2 * 3 + 4] = 0.0f;
			vertices[i * 2 * 3 + 5] = col - col / 2.0f;
		}
		for (int i = 0; i < col + 1; ++i)
		{
			int base = (row + 1) * 2 * 3;
			// vertex position (-z)
			vertices[base + i * 2 * 3 + 0] = row / 2.0f - row;
			vertices[base + i * 2 * 3 + 1] = 0.0f;
			vertices[base + i * 2 * 3 + 2] = col / 2.0f - col + i;
			// vertex position (+z)
			vertices[base + i * 2 * 3 + 3] = row - row / 2.0f;
			vertices[base + i * 2 * 3 + 4] = 0.0f;
			vertices[base + i * 2 * 3 + 5] = col / 2.0f - col + i;
		}

		uint* indices = new uint[verticesArraySize / 3];
		for (int i = 0; i < verticesArraySize / 3; ++i)
		{
			indices[i] = i;
		}

		vertexBuffer->SetData(vertices, sizeof(float) * verticesArraySize);
		indexBuffer->SetData(indices, verticesArraySize / 3);

		// free memory
		if (vertices) delete[] vertices;
		if (indices) delete[] indices;
		vertices = nullptr;
		indices = nullptr;

		vb.push_back(vertexBuffer);
		ib.push_back(indexBuffer);		
		return new VertexArray(layout, vb, ib, PrimitiveType::LINES);
	}
    
}
