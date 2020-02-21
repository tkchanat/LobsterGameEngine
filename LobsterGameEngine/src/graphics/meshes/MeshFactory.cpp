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
        VertexBuffer* vertexBuffer = new VertexBuffer();
		IndexBuffer* indexBuffer = new IndexBuffer();
		VertexLayout* layout = new VertexLayout();
        layout->Add<float>("in_position", 3);
		layout->Add<float>("in_normal", 3);
		layout->Add<float>("in_texcoord", 2);
		layout->Add<float>("in_tangent", 3);
		layout->Add<float>("in_bitangent", 3);

		const int VERTICES_PER_FACE = 36;
		const int STRIDE = 14;
		float vertices[VERTICES_PER_FACE * STRIDE] = {
			// position	       // normal		  // texture    // tangent		   // bitangent
			-1.0, -1.0, 1.0,   0.0, 0.0, 1.0,     0.0, 1.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			1.0, -1.0, 1.0,    0.0, 0.0, 1.0,     1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, 1.0,    0.0, 0.0, 1.0,     0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, 1.0,    0.0, 0.0, 1.0,     0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			1.0, -1.0, 1.0,    0.0, 0.0, 1.0,     1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			1.0, 1.0, 1.0,     0.0, 0.0, 1.0,     1.0, 0.0,     1.0, 0.0, 0.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, 1.0,    0.0, 1.0, 0.0,     0.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			1.0, 1.0, 1.0,     0.0, 1.0, 0.0,     1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			-1.0, 1.0, -1.0,   0.0, 1.0, 0.0,     0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			-1.0, 1.0, -1.0,   0.0, 1.0, 0.0,     0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			1.0, 1.0, 1.0,     0.0, 1.0, 0.0,     1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			1.0, 1.0, -1.0,    0.0, 1.0, 0.0,     1.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, -1.0,
			-1.0, 1.0, -1.0,   0.0, 0.0, -1.0,    1.0, 0.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			1.0, 1.0, -1.0,    0.0, 0.0, -1.0,    0.0, 0.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			-1.0, -1.0, -1.0,  0.0, 0.0, -1.0,    1.0, 1.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			-1.0, -1.0, -1.0,  0.0, 0.0, -1.0,    1.0, 1.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			1.0, 1.0, -1.0,    0.0, 0.0, -1.0,    0.0, 0.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			1.0, -1.0, -1.0,   0.0, 0.0, -1.0,    0.0, 1.0,     -1.0, 0.0, 0.0,    0.0, 1.0, 0.0,
			-1.0, -1.0, -1.0,  0.0, -1.0, 0.0,    0.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			1.0, -1.0, -1.0,   0.0, -1.0, 0.0,    1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			-1.0, -1.0, 1.0,   0.0, -1.0, 0.0,    0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			-1.0, -1.0, 1.0,   0.0, -1.0, 0.0,    0.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			1.0, -1.0, -1.0,   0.0, -1.0, 0.0,    1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			1.0, -1.0, 1.0,    0.0, -1.0, 0.0,    1.0, 0.0,     1.0, 0.0, 0.0,     0.0, 0.0, 1.0,
			1.0, -1.0, 1.0,    1.0, 0.0, 0.0,     0.0, 1.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			1.0, -1.0, -1.0,   1.0, 0.0, 0.0,     1.0, 1.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			1.0, 1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			1.0, 1.0, 1.0,     1.0, 0.0, 0.0,     0.0, 0.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			1.0, -1.0, -1.0,   1.0, 0.0, 0.0,     1.0, 1.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			1.0, 1.0, -1.0,    1.0, 0.0, 0.0,     1.0, 0.0,     0.0, 0.0, -1.0,    0.0, 1.0, 0.0,
			-1.0, -1.0, -1.0,  -1.0, 0.0, 0.0,    0.0, 1.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0,
			-1.0, -1.0, 1.0,   -1.0, 0.0, 0.0,    1.0, 1.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, -1.0,   -1.0, 0.0, 0.0,    0.0, 0.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, -1.0,   -1.0, 0.0, 0.0,    0.0, 0.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0,
			-1.0, -1.0, 1.0,   -1.0, 0.0, 0.0,    1.0, 1.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0,
			-1.0, 1.0, 1.0,    -1.0, 0.0, 0.0,    1.0, 0.0,     0.0, 0.0, 1.0,     0.0, 1.0, 0.0
		};

		uint indices[36] = {
			0,1,2,		3,4,5,
			6,7,8,		9,10,11,
			12,13,14,	15,16,17,
			18,19,20,	21,22,23,
			24,25,26,	27,28,29,
			30,31,32,	33,34,35
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
		layout->Add<float>("in_position", 3);
		layout->Add<float>("in_normal", 3);
		layout->Add<float>("in_texcoord", 2);

		float vertices[48] = {
			// positions		 // normals			// texCoords
			-1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
						  
			-1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f
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
	VertexArray * MeshFactory::Sphere(float radius, int du, int dv)
	{
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
		layout->Add<float>("in_position", 3);
		layout->Add<float>("in_normal", 3);
		layout->Add<float>("in_texcoord", 2);
		layout->Add<float>("in_tangent", 3);
		layout->Add<float>("in_bitangent", 3);
		const int stride = 14;

		float* vertices = new float[(du+1) * (dv+1) * stride];
		float* vtx = vertices;
		uint* indices = new uint[du * (dv-1) * 6];
		uint* ind = indices;

		float x, y, z, xy;                              // vertex position
		float lengthInv = 1.0f / radius;    // vertex normal
		float sectorStep = 2 * M_PI / du;
		float stackStep = M_PI / dv;
		float sectorAngle, stackAngle;
		// Add vertices
		for (int i = 0; i <= dv; ++i)
		{
			stackAngle = M_PI / 2 - i * stackStep;	// starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);			// r * cos(u)
			z = radius * sinf(stackAngle);			// r * sin(u)
			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= du; ++j)
			{
				sectorAngle = j * sectorStep; // starting from 0 to 2pi
				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
				*vtx++ = x;
				*vtx++ = y;
				*vtx++ = z;
				// normalized vertex normal (nx, ny, nz)
				*vtx++ = x * lengthInv;
				*vtx++ = y * lengthInv;
				*vtx++ = z * lengthInv;
				// vertex tex coord (s, t) range between [0, 1]
				*vtx++ = (float)j / du;
				*vtx++ = (float)i / dv;
				// tangent & bitangent
				*vtx++ = 0.0f;
				*vtx++ = 0.0f;
				*vtx++ = 0.0f;
				*vtx++ = 0.0f;
				*vtx++ = 0.0f;
				*vtx++ = 0.0f;
			}
		}
		// Calculate tangent & bitangent
		for (int i = 0; i < (du + 1) * (dv + 1) * stride; i += 3*stride) {
			// every triangle, use info from the three vertices
			glm::vec3 pos1 = glm::vec3(vertices[i+0], vertices[i+1], vertices[i+2]);
			glm::vec3 pos2 = glm::vec3(vertices[i+stride+0], vertices[i+stride+1], vertices[i+stride+2]);
			glm::vec3 pos3 = glm::vec3(vertices[i+2*stride+0], vertices[i+2*stride+1], vertices[i+2*stride+2]);
			glm::vec2 uv1 = glm::vec2(vertices[i+6], vertices[i+7]);
			glm::vec2 uv2 = glm::vec2(vertices[i+stride+6], vertices[i+stride+7]);
			glm::vec2 uv3 = glm::vec2(vertices[i+2*stride+6], vertices[i+2*stride+7]);
			glm::vec3 edge1 = pos2 - pos1;
			glm::vec3 edge2 = pos3 - pos1;
			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;
			glm::vec3 tangent(0);
			glm::vec3 bitangent(0);
			const float epsilon = 0.00000001f;
			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + epsilon);
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent = glm::normalize(tangent + epsilon);
			bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			bitangent = glm::normalize(bitangent + epsilon);
			// assign values
			for (int j = 0; j < 3; ++j) {
				vertices[i + j * stride + 8] = tangent.x;
				vertices[i + j * stride + 9] = tangent.y;
				vertices[i + j * stride + 10] = tangent.z;
				vertices[i + j * stride + 11] = bitangent.x;
				vertices[i + j * stride + 12] = bitangent.y;
				vertices[i + j * stride + 13] = bitangent.z;
			}
		}

		// Add indices
		int k1, k2;
		for (int i = 0; i < dv; ++i)
		{
			k1 = i * (du + 1);     // beginning of current stack
			k2 = k1 + du + 1;      // beginning of next stack
			for (int j = 0; j < du; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0)
				{
					*ind++ = k1;
					*ind++ = k2;
					*ind++ = k1 + 1;
				}
				// k1+1 => k2 => k2+1
				if (i != (dv - 1))
				{
					*ind++ = k1 + 1;
					*ind++ = k2;
					*ind++ = k2 + 1;
				}
			}
		}
		vertexBuffer->SetData(vertices, (du + 1) * (dv + 1) * stride * sizeof(float));
		indexBuffer->SetData(indices, du * (dv - 1) * 6);
		vb.push_back(vertexBuffer);
		ib.push_back(indexBuffer);
		// free memory
		if (vertices) delete[] vertices;
		if (indices) delete[] indices;
		vertices = nullptr;
		indices = nullptr;
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

	VertexArray * MeshFactory::Sprite()
	{
		std::vector<VertexBuffer*> vb;
		std::vector<IndexBuffer*> ib;
		VertexBuffer* vertexBuffer = new VertexBuffer;
		IndexBuffer* indexBuffer = new IndexBuffer;
		VertexLayout* layout = new VertexLayout;
		layout->Add<float>("in_data", 4);

		float vertices[24] = {
			// positions   // texcoords
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,

			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
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
    
}
