#include "pch.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"

namespace Lobster
{
    
    Material::Material(const char* path) :
		m_mode(MODE_OPAQUE),
		m_shader(nullptr),
		m_path(FileSystem::Path(path)),
		m_name(path),
		m_json(path),
		b_dirty(false)
    {
		// shader
		std::string shaderPath = m_json.getValue("Shader", "shaders/Phong.glsl");
        m_shader = !shaderPath.empty() ? ShaderLibrary::Use(shaderPath.c_str()) : nullptr;
		// rendering mode
		int mode = m_json.getValue("RenderingMode", 0);
		m_mode = (RenderingMode)mode;
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
            std::pair<std::string, Texture2D*> newPair = { blueprint.first, texture };
            m_textureUnits.push_back(newPair);
        }
    }
    
    Material::~Material()
    {
    }

	void Material::OnImGuiRender()
	{
		std::string headerLabel = fmt::format("Material{}: {}", b_dirty ? "*" : "", m_name);
		if (ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Save material pop up
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Save", "", false))
					SaveConfiguration();
				if (ImGui::MenuItem("Cancel", "", false))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImVec2 previewSize(40, 40);
			Texture2D* notFound = TextureLibrary::Placeholder();
            static std::string selectedTexture;
			// Shader
			ImGui::Text(("Shader: " + m_shader->GetName()).c_str());
			ImGui::Spacing();
			// Rendering Mode
			const char* modes[] = { "Opaque", "Transparent" };
			ImGui::PushItemWidth(110);
			RenderingMode prev_mode = m_mode;
			ImGui::Combo("Rendering Mode", (int*)&m_mode, modes, IM_ARRAYSIZE(modes));
			b_dirty |= prev_mode != m_mode;
			ImGui::PopItemWidth();
			ImGui::Spacing();
			// Texture2D
			for (auto pair : m_textureUnits)
			{
				std::string label = pair.first + ": (%s)";
				void* texture = pair.second ? pair.second->Get() : nullptr;
				ImGui::Text(label.c_str(), texture ? pair.second->GetName().c_str() : "Missing");
				ImGui::SameLine(ImGui::GetWindowWidth() - 60);
				// texture button
                ImGui::PushID(pair.first.c_str());
                if(ImGui::ImageButton(texture ? texture : notFound->Get(), previewSize))
                {
                    std::string path = FileSystem::OpenFileDialog();
					if(!path.empty())
                    {
                        SetTextureUnit(pair.first.c_str(), path.c_str());
                    }
                }
				// right-click to remove
				if (ImGui::BeginPopupContextItem())
				{
					ImGui::Text("Remove Texture?");
					ImGui::Separator();
					if (ImGui::Button("Remove")) {
						SetTextureUnit(pair.first.c_str(), nullptr);
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
                ImGui::PopID();
			}

			for (auto& uniformBufferData : m_uniformBufferData)
			{
				uniformBufferData.second->OnImGuiRender(m_name, b_dirty);
			}
		}
	}

	void Material::SaveConfiguration()
	{
		// save the material configurations accordingly
		m_json.setValue("Shader", m_shader->GetName());
		m_json.setValue("Texture2D", nlohmann::json());
		m_json.setValue("RenderingMode", (int)m_mode);
		m_json.setValue("UniformBuffer", nlohmann::json());
		for (auto& pair : m_textureUnits)
			m_json.setValue(("Texture2D." + pair.first).c_str(), (pair.second) ? pair.second->GetName() : "");
		for (auto& pair : m_uniformBufferData)
			m_json.setValue(("UniformBuffer." + pair.first).c_str(), pair.second->Serialize());
		b_dirty = false;
	}

	void Material::SetTextureUnit(const char * name, const char * texturePath)
	{
		uint32_t useTexture = (texturePath == nullptr) ? 0 : 1;
		for (auto& textureUnit : m_textureUnits)
		{
			if (textureUnit.first == name)
			{
				GetUniformBufferData(0)->SetData(("Use" + std::string(name)).c_str(), (void*)&useTexture);
				textureUnit.second = useTexture ? TextureLibrary::Use(texturePath) : nullptr;
            }
		}
		b_dirty = true;
	}

	// =======================================================
	// MaterialLibrary
	// =======================================================
	MaterialLibrary* MaterialLibrary::s_instance = nullptr;

	void MaterialLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("MaterialLibrary already existed!");
		}
		s_instance = new MaterialLibrary();
	}

	Material * MaterialLibrary::Use(const char * path)
	{
		for (Material* material : s_instance->m_materials)
		{
			if (material->GetName() == path)
			{
				return material;
			}
		}
		Material* newMaterial = new Material(path);
		s_instance->m_materials.push_back(newMaterial);
		return newMaterial;
	}

}
