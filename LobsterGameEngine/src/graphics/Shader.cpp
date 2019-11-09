#include "pch.h"
#include "Shader.h"
#include "system/FileSystem.h"
#include "graphics/UniformBuffer.h"

namespace Lobster
{
    
    Shader::Shader(const char* path):
		m_name(path),
        m_path(FileSystem::Path(path)),
		b_compileSuccess(false)
    {
		b_compileSuccess = Compile();
        if(!b_compileSuccess)
        {
            LOG("Couldn't compile shader {}", m_path);
        }
		ParseUniform();
		ParseTexture();
    }
    
    Shader::~Shader()
    {
        glDeleteProgram(m_id);
    }
    
    void Shader::Bind() const
    {
        glUseProgram(m_id);
    }
    
    void Shader::Unbind() const
    {
        
    }
    
    void Shader::Reload()
    {
        
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
        const char* vertexShaderSource = vs.c_str();
        const char* fragmentShaderSource = fs.c_str();
        
        // build and compile our shader program
        // ------------------------------------
        // vertex shader
        m_vsId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vsId, 1, &vertexShaderSource, NULL);
        glCompileShader(m_vsId);
        // check for shader compile errors
        int successVS;
        char infoLog[512];
        glGetShaderiv(m_vsId, GL_COMPILE_STATUS, &successVS);
        if (!successVS)
        {
            glGetShaderInfoLog(m_vsId, 512, NULL, infoLog);
            LOG("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", infoLog);
        }
        // fragment shader
        m_fsId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_fsId, 1, &fragmentShaderSource, NULL);
        glCompileShader(m_fsId);
        // check for shader compile errors
		int successFS;
        glGetShaderiv(m_fsId, GL_COMPILE_STATUS, &successFS);
        if (!successFS)
        {
            glGetShaderInfoLog(m_fsId, 512, NULL, infoLog);
            LOG("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n{}", infoLog);
        }
        // link shaders
        m_id = glCreateProgram();
        glAttachShader(m_id, m_vsId);
        glAttachShader(m_id, m_fsId);
        glLinkProgram(m_id);
        // check for linking errors
		int successLink;
        glGetProgramiv(m_id, GL_LINK_STATUS, &successLink);
        if (!successLink) {
            glGetProgramInfoLog(m_id, 512, NULL, infoLog);
            LOG("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", infoLog);
        }

        glDeleteShader(m_vsId);
        glDeleteShader(m_fsId);

		if (!successVS || !successFS || !successLink)
		{
			return false;
		}
        
        return true;
    }

	void Shader::ParseUniform()
	{
		std::string source = FileSystem::ReadText(m_path.c_str());
		std::string block;
		size_t readLocation = 0;
		while ((readLocation = source.find("ubo_", readLocation)) != std::string::npos)
		{
			// get uniform block extract
			block = source.substr(readLocation);
			readLocation = source.find('}', readLocation);
			block = StringOps::substr(block, nullptr, "}");
			// parse uniform name
			std::string uniformBufferName = StringOps::substr(block, nullptr, "{");
			StringOps::Erase(uniformBufferName, " \t\n");
			block = block.substr(block.find('{') + 1);
			// parse individual elements
			StringOps::Erase(block, "{}\n\t");
			std::vector<std::string> elements = StringOps::split(block, ';');
			for (std::string& element : elements)
			{
				element = StringOps::Trim(element);
			}
			// store the raw info
			m_uniformBlueprints.push_back({ uniformBufferName, elements });
		}
	}

	void Shader::ParseTexture()
	{
		std::string source = FileSystem::ReadText(m_path.c_str());
		std::string block;

		// read texture 2d first
		size_t readLocation = 0;
		while ((readLocation = source.find("sampler2D", readLocation)) != std::string::npos)
		{
			// get uniform block extract
			block = source.substr(readLocation);
			readLocation = source.find(';', readLocation);
			block = StringOps::substr(block, nullptr, ";");
			std::string textureName = StringOps::split(StringOps::Trim(block), ' ')[1];
			m_texture2DSlots.push_back(glGetUniformLocation(m_id, textureName.c_str()));
			m_textureBlueprints.push_back({ textureName, "" });
		}

		// reset readLocation and read texture cube
		readLocation = 0;
		while ((readLocation = source.find("samplerCube", readLocation)) != std::string::npos)
		{
			// get uniform block extract
			block = source.substr(readLocation);
			readLocation = source.find(';', readLocation);
			block = StringOps::substr(block, nullptr, ";");
			std::string textureName = StringOps::split(StringOps::Trim(block), ' ')[1];
			m_texture2DSlots.push_back(glGetUniformLocation(m_id, textureName.c_str()));
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
    
	void Shader::SetTexture2D(int slot, void* texture2D)
	{
		if (slot < 0) return;
		int location = slot < m_texture2DSlots.size() ? m_texture2DSlots[slot] : -1;
		if (location == -1) return;
		glUseProgram(m_id);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, (intptr_t)texture2D);
		glUniform1i(location, slot);
	}

	void Shader::SetTextureCube(int slot, void * textureCube)
	{
		if (slot < 0) return;
		int location = slot < m_textureCubeSlots.size() ? m_textureCubeSlots[slot] : -1;
		if (location == -1) return;
		glUseProgram(m_id);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, (intptr_t)textureCube);
		glUniform1i(location, slot);
	}

	bool Shader::HasUniformBufferName(const char * name) const
	{
		for (auto& uniformBlueprint : m_uniformBlueprints)
		{
			if (uniformBlueprint.first == name)
			{
				return true;
			}
		}
		return false;
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
		return newShader;
	}

	void ShaderLibrary::SetBlockBinding(const char * name, int bindingPoint)
	{
		for (Shader* shader : s_instance->m_shaders)
		{
			shader->SetBlockBinding(name, bindingPoint);
		}
	}


}
