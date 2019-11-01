#include "pch.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"

namespace Lobster
{
    
    Material::Material(const char* path) :
		m_shader(nullptr),
		m_path(FileSystem::Path(path)),
		m_name(path),
		m_json(path)
    {
		// shader
		std::string shaderPath = m_json.getValue("Shader", "shaders/Phong.glsl");
        m_shader = !shaderPath.empty() ? ShaderLibrary::Use(shaderPath.c_str()) : nullptr;
		// initialize uniform buffer data
		auto& uniformBlueprints = m_shader->GetUniformBlueprints();
		for (auto& blueprint : uniformBlueprints)
		{
			UniformBufferData* dataBlock = new UniformBufferData(blueprint.first, blueprint.second);
			m_uniformBufferData.push_back({ blueprint.first, dataBlock });
			// read stored uniform buffer data, set to default value if none
			for (auto& element : blueprint.second)
			{
				std::vector<std::string> tokens = StringOps::split(element, ' ');
				size_t typeCount = UniformBufferDeclaration::ElementMap(UniformBufferDeclaration::EnumMap(tokens[0]));
				std::string attribute = "UniformBuffer." + blueprint.first + '.' + tokens[1];
				std::vector<int> intArray = m_json.getValue(attribute.c_str(), std::vector<int>(typeCount, 0));
				std::vector<float> floatArray = m_json.getValue(attribute.c_str(), std::vector<float>(typeCount, 1.0));
				void* data = UniformBufferDeclaration::TypeMap(tokens[0]) == UniformBufferDeclaration::INT ? (void*)intArray.data() : (void*)floatArray.data();
				dataBlock->SetData(tokens[1].c_str(), data);
			}
		}
        // textures
        auto& textureBlueprints = m_shader->GetTextureBlueprints();
        for (auto& blueprint : textureBlueprints)
        {
            if (blueprint.first.empty()) continue;
            std::string texturePath = m_json.getValue(std::string("Texture2D." + blueprint.first).c_str(), "");
            Texture2D* texture = texturePath.empty() ? nullptr : TextureLibrary::Use(texturePath.c_str());
            bool useTexture = texture ? true : false;
            GetUniformBufferData(0)->SetData(("Use" + blueprint.first).c_str(), (void*)&useTexture);
            std::pair<std::string, Texture2D*> newPair = { blueprint.first, texture };
            m_textureUnits.push_back(newPair);
        }
        
        m_fileBrowser.SetPwd(FileSystem::Path("textures"));
    }
    
    Material::~Material()
    {
		// save the material configurations accordingly
		m_json.setValue("Shader", m_shader->GetName());
		m_json.setValue("Texture2D", nlohmann::json());
		m_json.setValue("UniformBuffer", nlohmann::json());
		for (auto& pair : m_textureUnits)
			m_json.setValue(("Texture2D." + pair.first).c_str(), (pair.second) ? pair.second->GetName() : "");
		for (auto& pair : m_uniformBufferData)
			m_json.setValue(("UniformBuffer." + pair.first).c_str(), pair.second->Serialize());
    }

	void Material::OnImGuiRender()
	{
		std::string materialName = "Material: " + m_name;
		if (ImGui::CollapsingHeader(materialName.c_str()))
		{
			ImVec2 previewSize(40, 40);
			Texture2D* notFound = TextureLibrary::Placeholder();
            static std::string selectedTexture;

			for (auto pair : m_textureUnits)
			{
				std::string label = pair.first + ": (%s)";
				void* texture = pair.second ? pair.second->Get() : nullptr;
				ImGui::Text(label.c_str(), texture ? pair.second->GetName().c_str() : "Missing");
				ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                ImGui::PushID(pair.first.c_str());
                if(ImGui::ImageButton(texture ? texture : notFound->Get(), previewSize))
                {
                    std::string path = FileSystem::OpenFileDialog();
					if(!path.empty())
                    {
                        SetTextureUnit(pair.first.c_str(), path.c_str());
                    }
                }
                ImGui::PopID();
			}

			for (auto& uniformBufferData : m_uniformBufferData)
			{
				uniformBufferData.second->OnImGuiRender(m_name);
			}
		}
	}

	void Material::SetTextureUnit(const char * name, const char * texturePath)
	{
		for (auto& textureUnit : m_textureUnits)
		{
			if (textureUnit.first == name)
			{
                bool True = true;
                GetUniformBufferData(0)->SetData(("Use" + std::string(name)).c_str(), (void*)&True);
				textureUnit.second = TextureLibrary::Use(texturePath);
            }
		}
	}
    
}
