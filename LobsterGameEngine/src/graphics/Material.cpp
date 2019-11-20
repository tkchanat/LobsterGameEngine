#include "pch.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"

namespace Lobster
{
    
    Material::Material(const char* path) :
		m_mode(MODE_OPAQUE),
		m_shader(nullptr),
		m_name(path),
		m_json(path),
		m_uniformData(nullptr),
		m_uniformDataSize(0),
		b_dirty(false)
    {
		// shader
		std::string shaderPath = m_json.getValue("Shader", "shaders/Phong.glsl");
        m_shader = !shaderPath.empty() ? ShaderLibrary::Use(shaderPath.c_str()) : nullptr;
		// rendering mode
		int mode = m_json.getValue("RenderingMode", 0);
		m_mode = (RenderingMode)mode;
		// initialize uniform buffer data
		InitializeUniformsFromShader();
    }

	void Material::InitializeUniformsFromShader()
	{
		// deallocate previous buffer if any
		if (m_uniformData) {
			delete[] m_uniformData;
			m_uniformData = nullptr;
		}

		// create buffer
		const std::vector<UniformDeclaration>& declarations = m_shader->GetUniformDeclarations();
		for (auto decl : declarations) {
			m_uniformDataSize += decl.Size();
		}
		m_uniformData = new byte[m_uniformDataSize];
		memset(m_uniformData, 0, m_uniformDataSize);

		// allocate each texture into its unique slot
		size_t offset = 0;
		for (auto decl : declarations) {
			if (decl.Type == UniformDeclaration::SAMPLER2D) {
				uint slot = m_textures.size();
				memcpy(m_uniformData + offset, &slot, sizeof(uint));
				Texture2D* texture = nullptr;
				m_textures.push_back(texture);
			}
			offset += decl.Size();
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

			ImVec2 previewSize(32, 32);
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
			// Uniforms
			auto declaration = m_shader->GetUniformDeclarations();
			size_t offset = 0;
			for (auto decl : declaration) {
				const char* label = decl.Name.c_str();
				void* data = m_uniformData + offset;
				ImGui::PushID(label);
				switch (decl.Type) {
				case UniformDeclaration::BOOL:
					ImGui::Checkbox(label, (bool*)data); break;
				case UniformDeclaration::FLOAT:
					ImGui::SliderFloat(label, (float*)data, 0.f, 1.f); break;
				case UniformDeclaration::VEC3:
					ImGui::ColorEdit3(label, (float*)data); break;
				case UniformDeclaration::VEC4:
					ImGui::ColorEdit4(label, (float*)data); break;
				case UniformDeclaration::SAMPLER2D:
					if (ImGui::ImageButton(m_textures[*(uint*)data] ? m_textures[*(uint*)data]->Get() : notFound->Get(), previewSize)) {
						std::string path = FileSystem::OpenFileDialog();
						if (!path.empty()) {
							m_textures[*(uint*)data] = TextureLibrary::Use(path.c_str());
							b_dirty = true;
						}
					}
					if (ImGui::BeginPopupContextItem()) {
						ImGui::Text("Remove Texture?");
						ImGui::Separator();
						if (ImGui::Button("Remove")) {
							m_textures[*(uint*)data] = nullptr;
							b_dirty = true;
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}
					ImGui::SameLine();
					ImGui::Text(label);
					break;
				default: break;
				}
				ImGui::PopID();
				b_dirty |= ImGui::IsItemActive();
				offset += decl.Size();
			}

		}
	}

	void Material::SetUniforms()
	{
		auto declaration = m_shader->GetUniformDeclarations();
		size_t offset = 0;
		for (auto decl : declaration) {
			std::string label = decl.Name;
			byte* data = m_uniformData + offset;
			if (decl.Type == UniformDeclaration::SAMPLER2D) {
				Texture2D* texture = m_textures[*(uint*)data];
				m_shader->SetTexture2D(*(uint*)data, texture ? texture->Get() : nullptr);
			}
			m_shader->SetUniform(decl.Name.c_str(), decl.Type, data);
			offset += decl.Size();
		}
	}

	void Material::SaveConfiguration()
	{
		// save the material configurations accordingly
		//m_json.setValue("Shader", m_shader->GetName());
		//m_json.setValue("RenderingMode", (int)m_mode);

		//std::vector<std::string> paths;
		//for (Texture2D* texture : m_textures) paths.push_back(texture ? texture->GetName() : "");
		//m_json.setValue("Texture2D", paths);

		//std::string binary;
		//for (int i = 0; i < m_uniformDataSize; ++i) binary += m_uniformData[i];
		//m_json.setValue("Uniforms", binary);

		b_dirty = false;
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
