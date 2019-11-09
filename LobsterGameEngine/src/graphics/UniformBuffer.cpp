#include "pch.h"
#include "UniformBuffer.h"
#include "graphics/Shader.h"
#include "system/FileSystem.h"

namespace Lobster
{

	UniformBuffer::UniformBuffer(const char* name, int size, void * data) :
		m_name(name),
		m_id(0),
		m_size(size)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void UniformBuffer::SetData(void* data)
	{
		if (!data || !m_size) return;
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_size, data);
	}

	// =======================================================
	// UniformBufferLibrary ==================================
	// =======================================================
	UniformBufferLibrary* UniformBufferLibrary::s_instance = nullptr;

	void UniformBufferLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("UniformBufferLibrary already existed!");
		}
		s_instance = new UniformBufferLibrary();
	}

	UniformBuffer * UniformBufferLibrary::Use(const char * name, int size)
	{
		// find if it exists in library
		for (UniformBuffer* ubo : s_instance->m_uniformBuffers)
		{
			if (ubo->GetName() == name)
			{
				if(ubo->GetSize() != size) throw std::runtime_error("Existing UBO has the same name but a size mismatch!");
				return ubo;
			}
		}
		// create one if not found
		UniformBuffer* newUniformBuffer = new UniformBuffer(name, size);
		int bindingPoint = s_instance->m_uniformBuffers.size();
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, (intptr_t)newUniformBuffer->Get());
		ShaderLibrary::SetBlockBinding(name, bindingPoint); // notify all shaders that this uniform buffer is located at this bindingPoint
		s_instance->m_uniformBuffers.push_back(newUniformBuffer);
		return newUniformBuffer;
	}

	UniformBufferData::UniformBufferData(const std::string& blueprintName, const std::vector<std::string>& blueprintElements) :
		m_buffer(nullptr),
		m_bufferData(nullptr)
	{
		// parse raw info into UniformBufferDeclaration(s)
		size_t bufferSize = 0;
		const std::string& uniformBufferName = blueprintName;
		const std::vector<std::string>& uniformBufferElements = blueprintElements;
		for (int i = 0; i < uniformBufferElements.size(); ++i)
		{
			const std::string& element = uniformBufferElements[i];
			std::vector<std::string> tokens = StringOps::split(element, ' ');
			if(tokens.size() < 2) throw std::runtime_error("WTF? This shouldn't be happening!!");
			std::string& dataType = tokens[0];
			std::string& name = tokens[1];
			size_t size = UniformBufferDeclaration::SizeMap(UniformBufferDeclaration::EnumMap(dataType));
			m_declaration.push_back({ name, bufferSize, UniformBufferDeclaration::EnumMap(dataType) });
			bufferSize += size;
		}
		// get the handle to the buffer object, create one if not
		m_buffer = UniformBufferLibrary::Use(uniformBufferName.c_str(), bufferSize);

		// create buffer data
		m_bufferData = new byte[bufferSize];
		memset(m_bufferData, 0, bufferSize);
	}

	void UniformBufferData::BindData()
	{
		m_buffer->SetData(m_bufferData);
	}

	void UniformBufferData::OnImGuiRender(const std::string& materialName, bool& dirty)
	{
		// display values
		for (UniformBufferDeclaration& decl : m_declaration)
		{
			// escape padding variables
			if (decl.Name.find("pad") != std::string::npos) continue;
            if (decl.Name.find("Use") != std::string::npos) continue;
			// render field
			void* data = m_bufferData + decl.Offset;
			std::string label = decl.Name + "##" + materialName;
			switch (decl.Type)
			{
			case UniformBufferDeclaration::BOOL:
				ImGui::Checkbox(label.c_str(), (bool*)data);
				break;
			case UniformBufferDeclaration::FLOAT:
				ImGui::SliderFloat(label.c_str(), (float*)data, 0.f, 1.f);
				break;
			case UniformBufferDeclaration::VEC3:
				ImGui::ColorEdit3(label.c_str(), (float*)data);
				break;
			case UniformBufferDeclaration::VEC4:
				ImGui::ColorEdit4(label.c_str(), (float*)data);
				break;
			default:
				break;
			}
			dirty |= ImGui::IsItemActive();
		}
	}

	void UniformBufferData::SetData(const char * name, void * data)
	{
		auto it = std::find_if(m_declaration.begin(), m_declaration.end(), [name](const UniformBufferDeclaration& e) { return e.Name == name; });
		if (it == m_declaration.end()) return;
		memcpy(m_bufferData + it->Offset, data, UniformBufferDeclaration::SizeMap(it->Type));
	}

	nlohmann::json UniformBufferData::Serialize() const
	{
		auto floatString = [](void* data) {
			float value = *(float*)data;
			return std::to_string(roundf(value * 10000) / 10000); // round to 4 decimal place
		};
		std::string output = "{";
		for (int i = 0; i < m_declaration.size(); ++i)
		{
			const UniformBufferDeclaration& uniform = m_declaration[i];
			output += "\"" + uniform.Name + "\": [";
			int elements = UniformBufferDeclaration::ElementMap(uniform.Type);
			bool isBool = uniform.Type == UniformBufferDeclaration::BOOL;
			bool isInt = uniform.Type == UniformBufferDeclaration::INT || uniform.Type == UniformBufferDeclaration::IVEC2 || uniform.Type == UniformBufferDeclaration::IVEC3 || uniform.Type == UniformBufferDeclaration::IVEC4;
			bool isFloat = uniform.Type == UniformBufferDeclaration::FLOAT || uniform.Type == UniformBufferDeclaration::VEC2 || uniform.Type == UniformBufferDeclaration::VEC3 || uniform.Type == UniformBufferDeclaration::VEC4;
			if (isBool)
			{
				// one boolean value at this moment
				output += (*(bool*)(m_bufferData + uniform.Offset)) ? "true" : "false";
			}
			if (isInt)
			{
				for (int j = 0; j < elements; ++j)
					output += (j < elements - 1) ? std::to_string(*(int*)(m_bufferData + uniform.Offset + sizeof(int) * j)) + ", " : std::to_string(*(int*)(m_bufferData + uniform.Offset + sizeof(int) * j));
			}
			if (isFloat)
			{
				for (int j = 0; j < elements; ++j)
					output += (j < elements - 1) ? floatString(m_bufferData + uniform.Offset + sizeof(float) * j) + ", " : floatString(m_bufferData + uniform.Offset + sizeof(float) * j);
			}
			output += (i < m_declaration.size() - 1) ? "], " : "]";
		}
		output += "}";
		return nlohmann::json::parse(output);
	}

}
