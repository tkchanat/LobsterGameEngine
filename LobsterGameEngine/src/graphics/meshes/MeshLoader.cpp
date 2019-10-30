#include "pch.h"
#include "MeshLoader.h"

#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/VertexLayout.h"
#include "graphics/IndexBuffer.h"
#include "graphics/Texture.h"

//  Assimp include
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>

namespace Lobster
{

	//  Helper function declaration
	void processMesh(aiMesh *mesh, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, glm::vec3& min, glm::vec3& max);
	void processNode(aiNode *node, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, glm::vec3& min, glm::vec3& max);

	//======================================================
	//  Static functions
	//======================================================    

    std::pair<glm::vec3, glm::vec3> MeshLoader::Load(const char* path, std::vector<VertexArray*>& vertexArrays, std::vector<Material*>& materialArrays)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
			aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs
        );
        
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(-std::numeric_limits<float>::max());
        
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG(import.GetErrorString());
            return { glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) };
        }

		// group meshes together using same materials
		assert(scene->mNumMaterials > 0 && "Key: I will fix this bug later, don't import this model");
		vertexArrays.resize(scene->mNumMaterials);
		materialArrays.resize(scene->mNumMaterials);

		std::vector<std::vector<VertexBuffer*>> vertexBuffers(scene->mNumMaterials);
		std::vector<std::vector<IndexBuffer*>> indexBuffers(scene->mNumMaterials);
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		layout->Add<float>("in_normal", 3, sizeof(float) * 3);
		layout->Add<float>("in_texcoord", 2, sizeof(float) * 6);
		layout->Add<float>("in_tangent", 3, sizeof(float) * 8);
		layout->Add<float>("in_bitangent", 3, sizeof(float) * 11);

		//INFO("Importing {}", path);
        processNode(scene->mRootNode, scene, vertexBuffers, indexBuffers, min, max);

		for (int i = 0; i < scene->mNumMaterials; ++i)
		{
			// finalize vertex arrays
			vertexArrays[i] = new VertexArray(layout, vertexBuffers[i], indexBuffers[i], PrimitiveType::TRIANGLES);

			// this will allow our material files (.mat) to overwrite what is defined in the 3d object material configurations.
			if (materialArrays[i] != nullptr) continue;
			
			// finalize materials
			aiString materialName;
			aiColor3D diffuseColor, specularColor;
			aiString diffuseMap, normalMap;
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, materialName);
			scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
			scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
			scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseMap);
			scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &normalMap);

			std::vector<std::string> nameTokens = StringOps::split(path, '/');
			std::string meshName = StringOps::substr(nameTokens[nameTokens.size() - 1], nullptr, ".");
			std::string materialPath = "materials/" + std::string(materialName.C_Str());
			std::string diffuseMapPath = "textures/" + meshName + '/' + std::string(diffuseMap.C_Str());
			std::string normalMapPath  = "textures/" + meshName + '/' + std::string(normalMap.C_Str());
			Material* newMaterial = new Material(materialPath.c_str());
			newMaterial->GetUniformBufferData(0)->SetData("DiffuseColor", (void*)&diffuseColor);
			newMaterial->GetUniformBufferData(0)->SetData("SpecularColor", (void*)&specularColor);

			if (diffuseMap.length) {
				newMaterial->SetTextureUnit("DiffuseMap", diffuseMapPath.c_str());
			}
			if (normalMap.length) {
				newMaterial->SetTextureUnit("NormalMap", normalMapPath.c_str());
			}
			
			materialArrays[i] = newMaterial;
		}
        
        return { min, max };
    }
    
	//======================================================
	//  Helper functions
	//======================================================

	void processMesh(aiMesh *mesh, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, glm::vec3& min, glm::vec3& max)
    {
        VertexBuffer* vb = new VertexBuffer();
        IndexBuffer* ib = new IndexBuffer();
		int stride = 14;

        //	Process vertices
		const uint numVertices = stride * mesh->mNumVertices;
        float* vbData = new float[numVertices];    //  Hard-code
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
			vbData[i * stride + 0] = mesh->mVertices[i].x;
			vbData[i * stride + 1] = mesh->mVertices[i].y;
			vbData[i * stride + 2] = mesh->mVertices[i].z;
			vbData[i * stride + 3] = mesh->mNormals[i].x;
			vbData[i * stride + 4] = mesh->mNormals[i].y;
			vbData[i * stride + 5] = mesh->mNormals[i].z;
			vbData[i * stride + 6] = (mesh->mTextureCoords[0]) ? mesh->mTextureCoords[0][i].x : 0.0f;
			vbData[i * stride + 7] = (mesh->mTextureCoords[0]) ? mesh->mTextureCoords[0][i].y : 0.0f;
			vbData[i * stride + 8] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].x : 0.0f;
			vbData[i * stride + 9] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].y : 0.0f;
			vbData[i * stride + 10] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].z : 0.0f;
			vbData[i * stride + 11] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].x : 0.0f;
			vbData[i * stride + 12] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].y : 0.0f;
			vbData[i * stride + 13] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].z : 0.0f;
			//INFO("{:.1f}, {:.1f}, {:.1f},    {:.1f}, {:.1f}, {:.1f},    {:.1f}, {:.1f},", vbData[i * 8 + 0], vbData[i * 8 + 1], vbData[i * 8 + 2], vbData[i * 8 + 3], vbData[i * 8 + 4], vbData[i * 8 + 5], vbData[i * 8 + 6], vbData[i * 8 + 7]);
            min.x = mesh->mVertices[i].x < min.x ? mesh->mVertices[i].x : min.x;
            min.y = mesh->mVertices[i].y < min.y ? mesh->mVertices[i].y : min.y;
            min.z = mesh->mVertices[i].z < min.z ? mesh->mVertices[i].z : min.z;
            max.x = mesh->mVertices[i].x > max.x ? mesh->mVertices[i].x : max.x;
            max.y = mesh->mVertices[i].y > max.y ? mesh->mVertices[i].y : max.y;
            max.z = mesh->mVertices[i].z > max.z ? mesh->mVertices[i].z : max.z;
        }
        vb->SetData(vbData, numVertices * sizeof(float));
        
        //	Process indices
		const uint numIndices = 3 * mesh->mNumFaces;
        uint* ibData = new uint[numIndices];
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            ibData[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
            ibData[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
            ibData[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
			//INFO("{}, {}, {}", ibData[i * 3 + 0], ibData[i * 3 + 1], ibData[i * 3 + 2]);
        }
        ib->SetData(ibData, numIndices);

		//	Push vertex and index buffer into vectors
		assert(mesh->mMaterialIndex >= 0);
		vertexBuffers[mesh->mMaterialIndex].push_back(vb);
		indexBuffers[mesh->mMaterialIndex].push_back(ib);

		//	Release memory
		if (vbData)	delete[] vbData;
		vbData = nullptr;
		if (ibData)	delete[] ibData;
		ibData = nullptr;

    }
    
    void processNode(aiNode *node, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, glm::vec3& min, glm::vec3& max)
    {
        // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene, vertexBuffers, indexBuffers, min, max);
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, vertexBuffers, indexBuffers, min, max);
        }
    }
    
}
