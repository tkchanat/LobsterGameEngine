#include "pch.h"
#include "Shader.h"
#include "Material.h"
#include "components/LightComponent.h"
#include "system/FileSystem.h"
#include "graphics/Texture.h"

namespace Lobster
{
    
    Shader::Shader(const char* path) :
		m_id(glCreateProgram()),
		m_vsId(glCreateShader(GL_VERTEX_SHADER)),
		m_fsId(glCreateShader(GL_FRAGMENT_SHADER)),
		m_name(path),
        m_path(FileSystem::Path(path)),
		b_compileSuccess(false)
    {
		Reload();
    }
    
    Shader::~Shader()
    {
		glDeleteShader(m_vsId);
		glDeleteShader(m_fsId);
        glDeleteProgram(m_id);
    }
    
    void Shader::Bind() const
    {
        glUseProgram(m_id);
    }
    
    void Shader::Unbind() const
    {
		glUseProgram(0);
    }
    
    void Shader::Reload()
    {
		b_compileSuccess = Compile();
		if (!b_compileSuccess) {
			LOG("Couldn't compile shader {}", m_path);
			return;
		}
		ParseUniform();
		int block_index = glGetUniformBlockIndex(m_id, "ubo_Lights");
		glUniformBlockBinding(m_id, block_index, 1);
    }
    
    //  TODO:
    //  Rewrite this later, because this one is copied.
    bool Shader::Compile()
    {
        // Read VS and FS Source
		std::string source = FileSystem::ReadText(m_path.c_str());
		if (source.empty())
		{
			WARN("Shader file {} is either invalid or empty, aborting compilation...", m_path);
			return false;
		}
		std::string vs = StringOps::substr(source, "///VertexShader", "///FragmentShader");
		std::string fs = StringOps::substr(source, "///FragmentShader", nullptr);

		StringOps::ReplaceAll(vs, "///VertexShader", 
			R"(#version 410 core
uniform mat4 sys_world;
uniform mat4 sys_view;
uniform mat4 sys_projection;)");
		StringOps::ReplaceAll(fs, "///FragmentShader",
			R"(#version 410 core
#define TextureExists(tex) textureSize(##tex, 0).x > 1
#define PI 3.14159265359
#define MAX_DIRECTIONAL_LIGHTS )" + std::to_string(MAX_DIRECTIONAL_LIGHTS) + R"(
struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
	float padding;
};
layout (std140) uniform ubo_Lights {
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
	int padding;
} Lights;
uniform vec3 sys_cameraPosition;)");

        const char* vertexShaderSource = vs.c_str();
        const char* fragmentShaderSource = fs.c_str();
        
        char infoLog[512];
        int successVS, successFS, successLink;
        // vertex shader
		glShaderSource(m_vsId, 1, &vertexShaderSource, NULL);
        glCompileShader(m_vsId);
        glGetShaderiv(m_vsId, GL_COMPILE_STATUS, &successVS);
        if (!successVS) {
            glGetShaderInfoLog(m_vsId, 512, NULL, infoLog);
            INFO("Vertex Shader compilation failed: {}", infoLog);
        }
        // fragment shader
        glShaderSource(m_fsId, 1, &fragmentShaderSource, NULL);
        glCompileShader(m_fsId);
        glGetShaderiv(m_fsId, GL_COMPILE_STATUS, &successFS);
        if (!successFS) {
            glGetShaderInfoLog(m_fsId, 512, NULL, infoLog);
			INFO("Fragment Shader compilation failed: {}", infoLog);
        }
        // link shaders
        glAttachShader(m_id, m_vsId);
        glAttachShader(m_id, m_fsId);
        glLinkProgram(m_id);
        // check for linking errors
        glGetProgramiv(m_id, GL_LINK_STATUS, &successLink);
        if (!successLink) {
            glGetProgramInfoLog(m_id, 512, NULL, infoLog);
			INFO("Shader Linking failed: {}", infoLog);
        }

		if (!successVS || !successFS || !successLink) {
			return false;
		}
        
		INFO("{} successfully compiled!", m_name);
        return true;
    }

	void Shader::ParseUniform()
	{
		// clear first
		m_uniformDeclarations.clear();
		// parse
		std::string source = FileSystem::ReadText(m_path.c_str());
		std::string block;
		size_t readLocation = 0;
		while ((readLocation = source.find("uniform ", readLocation)) != std::string::npos) {
			size_t blockEnd = source.find(";", readLocation);
			block = source.substr(readLocation, blockEnd - readLocation);
			readLocation = blockEnd;
			std::vector<std::string> tokens = StringOps::split(block, ' ');
			std::string uniformType = tokens[1];
			std::string uniformName = tokens[2];
			m_uniformLocationMap[uniformName] = glGetUniformLocation(m_id, uniformName.c_str());
			m_uniformDeclarations.push_back(UniformDeclaration(uniformName, uniformType));
		}
	}

	void Shader::SetBlockBinding(const char * name, int bindingPoint)
	{
		uint blockIndex = glGetUniformBlockIndex(m_id, name);
		if (blockIndex == -1) return;
		glUniformBlockBinding(m_id, blockIndex, bindingPoint);
	}
    
    //------------------------------------------
    //  Function Overload
    //------------------------------------------
    
	void Shader::SetUniform(const char * name, UniformDeclaration::DataType type, byte * data)
	{
		if (m_uniformLocationMap.find(name) == m_uniformLocationMap.end()) return;
		int location = m_uniformLocationMap[name];
		switch (type)
		{
		case UniformDeclaration::INT:
		case UniformDeclaration::BOOL:
		case UniformDeclaration::SAMPLER2D:
		case UniformDeclaration::SAMPLER3D:
		case UniformDeclaration::SAMPLERCUBE:
			glUniform1iv(location, 1, (int*)data); break;
		case UniformDeclaration::FLOAT:
			glUniform1fv(location, 1, (float*)data); break;
		case UniformDeclaration::VEC2:
			glUniform2fv(location, 1, (float*)data); break;
		case UniformDeclaration::VEC3:
			glUniform3fv(location, 1, (float*)data); break;
		case UniformDeclaration::VEC4:
			glUniform4fv(location, 1, (float*)data); break;
		case UniformDeclaration::MAT3:
			glUniformMatrix3fv(location, 1, GL_FALSE, (float*)data); break;
		case UniformDeclaration::MAT4:
			glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data); break;
		default: break;
		}
	}

	void Shader::SetUniform(const char * name, const bool & data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		//glUniform3fv(location, 1, glm::value_ptr(data));
	}

	void Shader::SetUniform(const char * name, const glm::vec2 & data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform2fv(location, 1, glm::value_ptr(data));
	}

	void Shader::SetUniform(const char* name, const glm::vec3& data)
    {
        int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
        glUniform3fv(location, 1, glm::value_ptr(data));
    }

	void Shader::SetUniform(const char* name, const glm::vec4& data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform4fv(location, 1, glm::value_ptr(data));
	}
    
    void Shader::SetUniform(const char* name, const glm::mat4 &data)
    {
        int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

	void Shader::SetTexture2D(uint slot, void * texture2D)
	{
		glUseProgram(m_id);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, (intptr_t)texture2D);
	}
    
	void Shader::SetTextureCube(uint slot, void * textureCube)
	{
		glUseProgram(m_id);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, (intptr_t)textureCube);
	}

	size_t Shader::GetUniformBufferSize() const
	{
		size_t size = 0;
		for (auto decl : m_uniformDeclarations) {
			size += decl.Size();
		}
		return size;
	}

	// =======================================================
	// ShaderLibrary =========================================
	// =======================================================
	ShaderLibrary* ShaderLibrary::s_instance = nullptr;

	void ShaderLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("ShaderLibrary already existed!");
		}
		s_instance = new ShaderLibrary();
		EventDispatcher::AddCallback(EVENT_WINDOW_FOCUSED, new EventCallback<WindowFocusedEvent>([](WindowFocusedEvent* e) {
			if (e->Focused == true) {
				ShaderLibrary::LiveReload();
			}
		}));
	}

	void ShaderLibrary::LiveReload()
	{
		for (int i = 0; i < s_instance->m_shaders.size(); ++i)
		{
			Shader* shader = s_instance->m_shaders[i];
			std::filesystem::file_time_type newTimestamp = FileSystem::LastModified(shader->GetPath().c_str());
			if (newTimestamp != s_instance->m_shadersLastModified[i])
			{
				INFO("Live reloading {}...", shader->GetPath());
				shader->Reload();
				MaterialLibrary::ResizeUniformBuffer(shader);
				s_instance->m_shadersLastModified[i] = newTimestamp;
			}
		}
	}

	// use a specific shader by relative path
	Shader* ShaderLibrary::Use(const char* path)
	{
		for (Shader* shader : s_instance->m_shaders)
		{
			if (shader->GetName() == path)
			{
				return shader;
			}
		}
		Shader* newShader = new Shader(path);
		s_instance->m_shaders.push_back(newShader);
		s_instance->m_shadersLastModified.push_back(FileSystem::LastModified(path));
		return newShader;
	}

	//void ShaderLibrary::SetBlockBinding(const char * name, int bindingPoint)
	//{
	//	for (Shader* shader : s_instance->m_shaders)
	//	{
	//		shader->SetBlockBinding(name, bindingPoint);
	//	}
	//}


}
