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
		m_uniformData(nullptr),
		m_uniformDataSize(0),
		b_dirty(false)
    {
		// initialize all material data from binary
		std::stringstream ss = FileSystem::ReadStringStream(FileSystem::Path(m_name).c_str());
		bool matFileExist = false;
		try {
			cereal::BinaryInputArchive iarchive(ss);
			iarchive(*this);
			matFileExist = true;
		} 
		catch (std::exception e) {
			m_shader = ShaderLibrary::Use("shaders/Phong.glsl");
			LOG("{}", e.what());
		};

		m_uniformDataSize = m_shader->GetUniformBufferSize();
		if (m_uniformData == nullptr) m_uniformData = new byte[m_uniformDataSize];
		if (matFileExist == false) InitializeUniformsFromShader();
		AssignTextureSlot();
    }

	Material::Material(Shader * shader) :
		m_mode(MODE_OPAQUE),
		m_shader(shader),
		m_name("Raw Material From Shader"),
		m_uniformData(nullptr),
		m_uniformDataSize(0),
		b_dirty(false)
	{
		m_uniformDataSize = m_shader->GetUniformBufferSize();
		if (m_uniformData == nullptr) m_uniformData = new byte[m_uniformDataSize];
		InitializeUniformsFromShader();
		AssignTextureSlot();
	}

	void Material::InitializeUniformsFromShader()
	{
		// assume all uniform default values are zero
		if (m_uniformData == nullptr) return;
		memset(m_uniformData, 0, m_uniformDataSize);
		auto declaration = m_shader->GetUniformDeclarations();
		size_t offset = 0;
		for (auto decl : declaration) {
			if (!decl.DefaultValueString.empty())
			{
				std::vector<std::string> elements = StringOps::RegexAllOccurrence(decl.DefaultValueString, "[0-9.]+|true|false");
				switch (decl.Type)
				{
				case UniformDeclaration::BOOL: case UniformDeclaration::BVEC2: case UniformDeclaration::BVEC3: case UniformDeclaration::BVEC4:
					uint bbuf[4];
					for (int i = 0; i < elements.size(); ++i)
						bbuf[i] = elements[i] == "true" ? 1 : 0;
					memcpy(m_uniformData + offset, bbuf, decl.Size());
					break;
				case UniformDeclaration::UINT: case UniformDeclaration::UVEC2: case UniformDeclaration::UVEC3: case UniformDeclaration::UVEC4:
					uint ubuf[4];
					for (int i = 0; i < elements.size(); ++i)
						ubuf[i] = std::stoul(elements[i]);
					memcpy(m_uniformData + offset, ubuf, decl.Size());
					break;
				case UniformDeclaration::INT: case UniformDeclaration::IVEC2: case UniformDeclaration::IVEC3: case UniformDeclaration::IVEC4:
					int ibuf[4];
					for (int i = 0; i < elements.size(); ++i)
						ibuf[i] = std::stoi(elements[i]);
					memcpy(m_uniformData + offset, ibuf, decl.Size());
					break;
				case UniformDeclaration::FLOAT: case UniformDeclaration::VEC2: case UniformDeclaration::VEC3: case UniformDeclaration::VEC4:
					float fbuf[4];
					for (int i = 0; i < elements.size(); ++i)
						fbuf[i] = std::stof(elements[i]);
					memcpy(m_uniformData + offset, fbuf, decl.Size());
					break;
				case UniformDeclaration::MAT3: case UniformDeclaration::MAT4:
					break; // do nothing. we don't do initialization for matrix types for now.
				}
			}
			offset += decl.Size();
		}
	}

	void Material::AssignTextureSlot()
	{
		// assign each texture into its unique slot
		m_textures.clear();
		const std::vector<UniformDeclaration>& declarations = m_shader->GetUniformDeclarations();
		size_t offset = 0;
		for (auto decl : declarations) {
			if (decl.Type == UniformDeclaration::SAMPLER2D) {
				uint slot = (uint)m_textures.size();
				memcpy(m_uniformData + offset, &slot, sizeof(uint));
				m_textures.push_back(nullptr);
			}
			offset += decl.Size();
		}
		_textureNames.resize(m_textures.size());
		for (int i = 0; i < m_textures.size(); ++i) {
			m_textures[i] = _textureNames[i].empty() ? nullptr : TextureLibrary::Use(_textureNames[i].c_str());
		}
	}

	void Material::ResizeUniformBuffer(size_t newSize)
	{
		if (newSize == m_uniformDataSize) return;
		// create new buffer with new size
		byte* newBuffer = new byte[newSize];
		// copy the content from the old buffer
		memcpy(newBuffer, m_uniformData, std::min(newSize, m_uniformDataSize));
		// assign the new buffer size
		m_uniformDataSize = newSize;
		// removing old buffer
		if (m_uniformData) delete[] m_uniformData;
		m_uniformData = newBuffer;
	}
    
    Material::~Material()
    {
    }

	void Material::OnImGuiRender(int material_id)
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
			const auto findShaderIndex = [this](const char* shaders[], size_t size) -> int {
				for (int i = 0; i < size; ++i) {
					if (m_shader->GetName() == shaders[i])
						return i;
				}
				return 0;
			};
			const char* shaders[] = { "shaders/Phong.glsl", "shaders/PBR.glsl" };
			static int usedShader = findShaderIndex(shaders, sizeof(shaders));
			int prev_shader = usedShader;
			ImGui::Combo("Shader", &usedShader, shaders, IM_ARRAYSIZE(shaders));
			if (prev_shader != usedShader) {
				m_shader = ShaderLibrary::Use(shaders[usedShader]);
				ResizeUniformBuffer(m_shader->GetUniformBufferSize());
				AssignTextureSlot();
				b_dirty = true;
			}
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
				void* data = m_uniformData + offset;
				ImGui::PushID((decl.Name + std::to_string(material_id)).c_str());
				switch (decl.Type) {
				case UniformDeclaration::BOOL:
					ImGui::Checkbox(decl.Name.c_str(), (bool*)data); break;
				case UniformDeclaration::FLOAT:
					ImGui::SliderFloat(decl.Name.c_str(), (float*)data, 0.f, 1.f); break;
				case UniformDeclaration::VEC3:
					ImGui::ColorEdit3(decl.Name.c_str(), (float*)data); break;
				case UniformDeclaration::VEC4:
					ImGui::ColorEdit4(decl.Name.c_str(), (float*)data); break;
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
					ImGui::Text(decl.Name.c_str());
					break;
				default: break;
				}
				ImGui::PopID();
				b_dirty |= ImGui::IsItemActive();
				offset += decl.Size();
			}

		}
	}

	void Material::SetRawUniform(const char * name, void * data)
	{
		size_t offset = 0;
		auto declaration = m_shader->GetUniformDeclarations();
		for (auto decl : declaration) {
			if (decl.Name == name) {
				memcpy(m_uniformData + offset, data, decl.Size());
			}
			offset += decl.Size();
		}
	}

	void Material::SetUniforms()
	{
		if (m_uniformData == nullptr) return;
		auto declaration = m_shader->GetUniformDeclarations();
		size_t offset = 0;
		for (auto decl : declaration) {
			std::string label = decl.Name;
			byte* data = m_uniformData + offset;
			if (decl.Type == UniformDeclaration::SAMPLER2D) {
				uint slot = *(uint*)data;
				Texture2D* texture = m_textures[slot];
				m_shader->SetTexture2D(slot, texture ? texture->Get() : nullptr);
			}
			m_shader->SetUniform(decl.Name.c_str(), decl.Type, data);
			offset += decl.Size();
		}
	}

	void Material::SaveConfiguration()
	{
		for (int i = 0; i < m_textures.size(); ++i) _textureNames[i] = m_textures[i] ? m_textures[i]->GetName() : "";

		std::stringstream ss;
		{
			cereal::BinaryOutputArchive oachive(ss);
			oachive(*this);
		}
		FileSystem::WriteStringStream(FileSystem::Path(m_name).c_str(), ss);

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

	Material * MaterialLibrary::UseShader(const char * shaderPath)
	{
		return new Material(ShaderLibrary::Use(shaderPath));
	}

	void MaterialLibrary::ResizeUniformBuffer(Shader * shader)
	{
		for (Material* material : s_instance->m_materials) {
			if (material->GetShader() == shader) {
				material->ResizeUniformBuffer(shader->GetUniformBufferSize());
				material->AssignTextureSlot();
			}
		}
	}

}
