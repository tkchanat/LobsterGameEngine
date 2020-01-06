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

	void processMesh(aiMesh *mesh, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, MeshInfo& meshInfo);
	void processNode(aiNode *node, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, MeshInfo& meshInfo);
	void processBoneNode(aiNode * node, BoneNode& boneNode, const std::unordered_map<std::string, uint>& boneMap);

	//======================================================
	//  Static functions
	//======================================================    

    MeshInfo MeshLoader::Load(const char* path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
			aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs
        );
        
		// Initialize MeshInfo
		MeshInfo meshInfo;
		meshInfo.Bound.first = glm::vec3(std::numeric_limits<float>::max());
        meshInfo.Bound.second = glm::vec3(-std::numeric_limits<float>::max());
		bool hasAnimation = scene->mNumAnimations > 0;
        
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
            LOG(import.GetErrorString());
            return meshInfo;
        }

		// group meshes together using same materials
		std::vector<std::vector<VertexBuffer*>> vertexBuffers(scene->mNumMaterials);
		std::vector<std::vector<IndexBuffer*>> indexBuffers(scene->mNumMaterials);
		VertexLayout* layout = new VertexLayout();
		layout->Add<float>("in_position", 3);
		layout->Add<float>("in_normal", 3);
		layout->Add<float>("in_texcoord", 2);
		layout->Add<float>("in_tangent", 3);
		layout->Add<float>("in_bitangent", 3);
		if (hasAnimation) {
			layout->Add<int>("in_boneID", 4);
			layout->Add<float>("in_boneWeight", 4);
		}

        processNode(scene->mRootNode, scene, vertexBuffers, indexBuffers, meshInfo);
		processBoneNode(scene->mRootNode, meshInfo.RootNode, meshInfo.BoneMap);

		for (uint i = 0; i < scene->mNumMaterials; ++i)
		{
			// no vertex and index buffers use this material, skip it!
			if (vertexBuffers[i].empty() || indexBuffers[i].empty()) continue;

			// finalize vertex arrays
			meshInfo.Meshes.push_back(new VertexArray(layout, vertexBuffers[i], indexBuffers[i], PrimitiveType::TRIANGLES));

			// finalize materials
			aiString materialName;
			aiColor3D diffuseColor, specularColor;
			aiString diffuseMap, normalMap;
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, materialName);
			scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
			scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
			scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseMap);
			scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &normalMap);

			// don't give any material to this mesh, we'll define ours
			if (materialName == aiString(AI_DEFAULT_MATERIAL_NAME)) continue;

			std::vector<std::string> nameTokens = StringOps::split(path, '/');
			std::string meshName = StringOps::substr(nameTokens[nameTokens.size() - 1], nullptr, ".");
			std::string materialPath = "materials/" + std::string(materialName.C_Str()) + ".mat";
			std::string diffuseMapPath = "textures/" + meshName + '/' + std::string(diffuseMap.C_Str());
			std::string normalMapPath  = "textures/" + meshName + '/' + std::string(normalMap.C_Str());
			Material* newMaterial = MaterialLibrary::Use(materialPath.c_str());
			meshInfo.Materials.push_back(newMaterial);
		}

		// =======================================
		// Animations
		if (hasAnimation) {
			meshInfo.Animations.resize(scene->mNumAnimations);
			for (uint i = 0; i < scene->mNumAnimations; ++i) {
				aiAnimation* anim = scene->mAnimations[i];
				AnimationInfo& animInfo = meshInfo.Animations[i];
				animInfo.Name = anim->mName.data;
				animInfo.Duration = anim->mDuration;
				animInfo.TicksPerSecond = anim->mTicksPerSecond;
				animInfo.Channels.resize(anim->mNumChannels);
				for (uint j = 0; j < anim->mNumChannels; ++j) {
					aiNodeAnim* channel = anim->mChannels[j];
					ChannelInfo& channelInfo = animInfo.Channels[j];
					channelInfo.Name = channel->mNodeName.data;
					uint boneID = meshInfo.BoneMap[channelInfo.Name];
					animInfo.ChannelMap[boneID] = j;
					channelInfo.Position.resize(channel->mNumPositionKeys);
					channelInfo.Rotation.resize(channel->mNumRotationKeys);
					channelInfo.Scale.resize(channel->mNumScalingKeys);
					for (uint k = 0; k < channel->mNumPositionKeys; ++k) {
						aiVectorKey& positionKey = channel->mPositionKeys[k];
						channelInfo.Position[k].Time = positionKey.mTime;
						channelInfo.Position[k].Value = { positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z };
					}
					for (uint k = 0; k < channel->mNumRotationKeys; ++k) {
						aiQuatKey& rotationKey = channel->mRotationKeys[k];
						channelInfo.Rotation[k].Time = rotationKey.mTime;
						channelInfo.Rotation[k].Value = { rotationKey.mValue.w, rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z };
					}
					for (uint k = 0; k < channel->mNumScalingKeys; ++k) {
						aiVectorKey& scaleKey = channel->mScalingKeys[k];
						channelInfo.Scale[k].Time = scaleKey.mTime;
						channelInfo.Scale[k].Value = { scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z };
					}
				}
			}
		}

        return meshInfo;
    }
    
	//======================================================
	//  Helper functions
	//======================================================

	const int MAX_BONE_COUNT = 4;
	struct VertexBoneData {
		int IDs[MAX_BONE_COUNT];
		float Weights[MAX_BONE_COUNT];
	};

	inline glm::mat4 glmMatConversion(const aiMatrix4x4& from) 
	{
		glm::mat4 to = glm::mat4(1.0);
		to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
		to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
		to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
		to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;
		return to;
	};

	void processMesh(aiMesh *mesh, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, MeshInfo& meshInfo)
    {
        VertexBuffer* vb = new VertexBuffer();
        IndexBuffer* ib = new IndexBuffer();
		int stride = 14;

		// Process Bones
		VertexBoneData* boneData = nullptr;
		if (mesh->mNumBones > 0) {
			stride = 22;
			bool overMaxBoneCount = false;
			boneData = new VertexBoneData[mesh->mNumVertices];
			memset(boneData, 0, sizeof(VertexBoneData) * mesh->mNumVertices);
			for (int i = 0; i < mesh->mNumBones; ++i) {
				aiBone* bone = mesh->mBones[i];
				std::string name = bone->mName.data;
				if (meshInfo.BoneMap.find(name) == meshInfo.BoneMap.end()) {
					int nextBoneID = meshInfo.BoneTransforms.size();
					meshInfo.BoneMap[name] = nextBoneID; // Populate BoneMap
					glm::mat4 boneOffset = glmMatConversion(bone->mOffsetMatrix);
					meshInfo.BoneOffsets.push_back(boneOffset); // Set bone offset
					meshInfo.BoneTransforms.push_back(glm::mat4(1.0)); // Set default matrix
				}
				for (int j = 0; j < bone->mNumWeights; ++j) {
					int id = bone->mWeights[j].mVertexId; // Vertex index
					float weight = bone->mWeights[j].mWeight; // Weight exerted on vertex
					int k = 0;
					for (; k < MAX_BONE_COUNT; ++k) {
						if (boneData[id].Weights[k] == 0.0) {
							boneData[id].IDs[k] = i; // Bone index
							boneData[id].Weights[k] = weight;
							break;
						}
					}
					overMaxBoneCount |= k == MAX_BONE_COUNT;
				}
			}
			if (overMaxBoneCount) {
				WARN("Oops... Max bone influence reached. Animation may seems a little off.");
				for (uint i = 0; i < mesh->mNumVertices; ++i) {
					//LOG("boneData {}: [{},{},{},{}]\t[{},{},{},{}]", i, boneData[i].IDs[0], boneData[i].IDs[1], boneData[i].IDs[2], boneData[i].IDs[3],\
					//	boneData[i].Weights[0], boneData[i].Weights[1], boneData[i].Weights[2], boneData[i].Weights[3]);
					float sum = boneData[i].Weights[0] + boneData[i].Weights[1] + boneData[i].Weights[2] + boneData[i].Weights[3];
					float ratio = 1.f / sum;
					boneData[i].Weights[0] *= ratio;
					boneData[i].Weights[1] *= ratio;
					boneData[i].Weights[2] *= ratio;
					boneData[i].Weights[3] *= ratio;
					//LOG("weight sum {}: {}", i, boneData[i].Weights[0] + boneData[i].Weights[1] + boneData[i].Weights[2] + boneData[i].Weights[3]);
				}
			}

		}

        // Process Vertices
		struct VertexData {
			float position[3];
			float normal[3];
			float texcoord[2];
			float tangent[3];
			float bitangent[3];
			int boneId[4];
			float boneWeight[4];
		};
		VertexData* vbData = new VertexData[mesh->mNumVertices];
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
			vbData[i].position[0] = mesh->mVertices[i].x;
			vbData[i].position[1] = mesh->mVertices[i].y;
			vbData[i].position[2] = mesh->mVertices[i].z;
			vbData[i].normal[0] = mesh->mNormals[i].x;
			vbData[i].normal[1] = mesh->mNormals[i].y;
			vbData[i].normal[2] = mesh->mNormals[i].z;
			vbData[i].texcoord[0] = (mesh->mTextureCoords[0]) ? mesh->mTextureCoords[0][i].x : 0.0f;
			vbData[i].texcoord[1] = (mesh->mTextureCoords[0]) ? mesh->mTextureCoords[0][i].y : 0.0f;
			vbData[i].tangent[0] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].x : 0.0f;
			vbData[i].tangent[1] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].y : 0.0f;
			vbData[i].tangent[2] = (mesh->mTextureCoords[0]) ? mesh->mTangents[i].z : 0.0f;
			vbData[i].bitangent[0] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].x : 0.0f;
			vbData[i].bitangent[1] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].y : 0.0f;
			vbData[i].bitangent[2] = (mesh->mTextureCoords[0]) ? mesh->mBitangents[i].z : 0.0f;
			if (mesh->HasBones())
			{
				vbData[i].boneId[0] = boneData[i].IDs[0];
				vbData[i].boneId[1] = boneData[i].IDs[1];
				vbData[i].boneId[2] = boneData[i].IDs[2];
				vbData[i].boneId[3] = boneData[i].IDs[3];
				vbData[i].boneWeight[0] = boneData[i].Weights[0];
				vbData[i].boneWeight[1] = boneData[i].Weights[1];
				vbData[i].boneWeight[2] = boneData[i].Weights[2];
				vbData[i].boneWeight[3] = boneData[i].Weights[3];
			}

			//INFO("{:.1f}, {:.1f}, {:.1f},    {:.1f}, {:.1f}, {:.1f},    {:.1f}, {:.1f},", vbData[i * 8 + 0], vbData[i * 8 + 1], vbData[i * 8 + 2], vbData[i * 8 + 3], vbData[i * 8 + 4], vbData[i * 8 + 5], vbData[i * 8 + 6], vbData[i * 8 + 7]);
			glm::vec3& min = meshInfo.Bound.first;
			glm::vec3& max = meshInfo.Bound.second;
			min.x = mesh->mVertices[i].x < min.x ? mesh->mVertices[i].x : min.x;
            min.y = mesh->mVertices[i].y < min.y ? mesh->mVertices[i].y : min.y;
            min.z = mesh->mVertices[i].z < min.z ? mesh->mVertices[i].z : min.z;
            max.x = mesh->mVertices[i].x > max.x ? mesh->mVertices[i].x : max.x;
            max.y = mesh->mVertices[i].y > max.y ? mesh->mVertices[i].y : max.y;
            max.z = mesh->mVertices[i].z > max.z ? mesh->mVertices[i].z : max.z;
        }
        vb->SetData(vbData, mesh->mNumVertices * sizeof(VertexData));
        
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
		if (boneData) delete[] boneData;
		boneData = nullptr;
		if (vbData)	delete[] vbData;
		vbData = nullptr;
		if (ibData)	delete[] ibData;
		ibData = nullptr;

    }
    
    void processNode(aiNode *node, const aiScene *scene, std::vector<std::vector<VertexBuffer*>>& vertexBuffers, std::vector<std::vector<IndexBuffer*>>& indexBuffers, MeshInfo& meshInfo)
    {
        // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene, vertexBuffers, indexBuffers, meshInfo);
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, vertexBuffers, indexBuffers, meshInfo);
        }
    }
    
	void processBoneNode(aiNode * node, BoneNode& boneNode, const std::unordered_map<std::string, uint>& boneMap)
	{
		std::string nodeName = node->mName.data;
		auto it = boneMap.find(nodeName);
		boneNode.BoneID = it != boneMap.end() ? it->second : -1;
		boneNode.Matrix = glmMatConversion(node->mTransformation);

		// recursively process children bones
		if (node->mNumChildren == 0) return;
		boneNode.Children.resize(node->mNumChildren);
		for (int i = 0; i < node->mNumChildren; ++i) {
			processBoneNode(node->mChildren[i], boneNode.Children[i], boneMap);
		}
	}
}
