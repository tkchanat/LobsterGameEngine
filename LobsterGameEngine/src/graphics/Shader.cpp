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
		m_gsId(glCreateShader(GL_GEOMETRY_SHADER)),
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
		glDeleteShader(m_gsId);
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
			WARN("Shader file {} is either corrupted or empty, aborting compilation...", m_path);
			return false;
		}
		size_t readLocation = 0;
		std::string vs, gs, fs;
		while ((readLocation = source.find("///", readLocation)) != std::string::npos) {
			size_t blockEnd = source.find("\n", readLocation);
			std::string shaderType = source.substr(readLocation, blockEnd - readLocation);
			size_t nextShaderToken = source.find("///", blockEnd);
			std::string shaderCode = source.substr(readLocation, nextShaderToken - readLocation);
			if (shaderType == "///VertexShader")
				vs = shaderCode;
			else if (shaderType == "///GeometryShader")
				gs = shaderCode;
			else if (shaderType == "///FragmentShader")
				fs = shaderCode;
			readLocation = blockEnd;
		}

		// Preprocess system uniforms and shader version
		StringOps::ReplaceAll(vs, "///VertexShader", 
			R"(#version 410 core
uniform mat4 sys_world;
uniform mat4 sys_view;
uniform mat4 sys_projection;
uniform mat4 sys_bones[)"+ std::to_string(MAX_BONES) + R"(];
uniform bool sys_animate = false;
uniform vec3 sys_cameraPosition;)");

		StringOps::ReplaceAll(gs, "///GeometryShader", 
			R"(#version 410 core
uniform mat4 sys_world;
uniform mat4 sys_view;
uniform mat4 sys_projection;
uniform vec3 sys_cameraPosition;)");

		StringOps::ReplaceAll(fs, "///FragmentShader",
			R"(#version 410 core
#define PI 3.14159265359
#define EPSILON 0.0001
#define MAX_DIRECTIONAL_LIGHTS )" + std::to_string(MAX_DIRECTIONAL_LIGHTS) + R"(
#define MAX_POINT_LIGHTS )" + std::to_string(MAX_POINT_LIGHTS) + R"(
#define MAX_DIRECTIONAL_SHADOW )" + std::to_string(MAX_DIRECTIONAL_SHADOW) + R"(
struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
	float padding;
};
struct PointLight {
	vec3 position;
	float attenuation;
	vec3 color;
	float padding;
};
layout (std140) uniform ubo_Lights {
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight pointLights[MAX_POINT_LIGHTS];
	mat4 lightSpaceMatrix[MAX_DIRECTIONAL_SHADOW];
    int directionalLightCount;
	int pointLightCount;
} Lights;
uniform vec3 sys_cameraPosition;
uniform samplerCube sys_irradianceMap;
uniform samplerCube sys_prefilterMap;
uniform sampler2D sys_brdfLUTMap;
uniform sampler2D sys_shadowMap[MAX_DIRECTIONAL_SHADOW];)");

		// System defined macro
		fs = StringOps::RegexReplace(fs, "TextureExists\\((\\w*)\\)", "textureSize($1, 0).x > 1");
		b_hasGS = !gs.empty();

        const char* vertexShaderSource = vs.c_str();
		const char* geometryShaderSource = gs.c_str();
        const char* fragmentShaderSource = fs.c_str();
        
        char infoLog[512];
        int successVS, successGS, successFS, successLink;
		// vertex shader
		glShaderSource(m_vsId, 1, &vertexShaderSource, NULL);
		glCompileShader(m_vsId);
		glGetShaderiv(m_vsId, GL_COMPILE_STATUS, &successVS);
		if (!successVS) {
			glGetShaderInfoLog(m_vsId, 512, NULL, infoLog);
			INFO("Vertex Shader compilation failed: {}", infoLog);
		}
		// geometry shader
		if (b_hasGS)
		{
 			glShaderSource(m_gsId, 1, &geometryShaderSource, NULL);
			glCompileShader(m_gsId);
			glGetShaderiv(m_gsId, GL_COMPILE_STATUS, &successGS);
			if (!successGS) {
				glGetShaderInfoLog(m_gsId, 512, NULL, infoLog);
				INFO("Geometry Shader compilation failed: {}", infoLog);
			}
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
		if (b_hasGS) glAttachShader(m_id, m_gsId);
        glAttachShader(m_id, m_fsId);
        glLinkProgram(m_id);
        // check for linking errors
        glGetProgramiv(m_id, GL_LINK_STATUS, &successLink);
        if (!successLink) {
            glGetProgramInfoLog(m_id, 512, NULL, infoLog);
			INFO("Shader Linking failed: {}", infoLog);
        }

		if (!successVS || (b_hasGS && !successGS) || !successFS || !successLink) {
			WARN("Unable to compile {}", m_name);
			return false;
		}

		//INFO("{} successfully compiled!", m_name);
		//int total = -1;
		//glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &total);
		//for (int i = 0; i < total; ++i) {
		//	int name_len = -1, num = -1;
		//	GLenum type = GL_ZERO;
		//	char name[100];
		//	glGetActiveUniform(m_id, GLuint(i), sizeof(name) - 1,
		//		&name_len, &num, &type, name);
		//	name[name_len] = 0;
		//	GLuint location = glGetUniformLocation(m_id, name);
		//	LOG("{}: {} ({})", m_name, name, location);
		//}

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
			// default value
			std::string defaultValStr = "";
			if (block.find("=") != std::string::npos) {
				size_t parenthesesPos = block.find("(");
				defaultValStr = block.substr(parenthesesPos != std::string::npos ? parenthesesPos : block.find("=") + 1);
			}
			// range value
			size_t lineEnd = source.find("\n", blockEnd + 1);
			std::string rangeValStr = source.substr(blockEnd + 1, lineEnd - readLocation);
			StringOps::Erase(rangeValStr, " ");
			float min = 0.f;
			float max = 1.f;
			bool hasRangeValue = rangeValStr.find("//[") != std::string::npos;
			if (hasRangeValue && (uniformType == "float" || uniformType == "double" || uniformType == "int" || uniformType == "uint")) {
				std::vector<std::string> range = StringOps::RegexAllOccurrence(rangeValStr, "[0-9]+");
				if (range.size() == 2) {
					min = std::stof(range[0]);
					max = std::stof(range[1]);
				}
			}

			m_uniformLocationMap[uniformName] = glGetUniformLocation(m_id, uniformName.c_str());
			m_uniformDeclarations.push_back(UniformDeclaration(uniformName, defaultValStr, uniformType, min, max));
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

	void Shader::SetUniform(const char * name, int data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform1i(location, data);
	}

	void Shader::SetUniform(const char * name, float data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform1f(location, data);
	}

	void Shader::SetUniform(const char * name, bool data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform1i(location, data);
	}

	void Shader::SetUniform(const char * name, const glm::ivec2 & data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniform2iv(location, 1, glm::value_ptr(data));
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
 
	void Shader::SetUniform(const char* name, const glm::mat3 &data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) {
			return;
		}
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(data));
	}

    void Shader::SetUniform(const char* name, const glm::mat4 &data)
    {
        int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

	void Shader::SetUniform(const char * name, size_t count, const glm::mat4 * data)
	{
		int location = glGetUniformLocation(m_id, name);
		if (location == -1) return;
		glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(*data));
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
				INFO("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
				INFO("Live reloading {}...", shader->GetName());
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
		s_instance->m_shadersLastModified.push_back(FileSystem::LastModified(newShader->GetPath().c_str()));
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
