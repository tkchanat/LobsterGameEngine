#include "pch.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "layer/EditorLayer.h"
#include "objects/GameObject.h"
#include "system/UndoSystem.h"

namespace Lobster
{
	const char* Material::shaders[] = { "Phong Shader", "PBR Shader" };
	const char* Material::shaderPath[] = { "shaders/Phong.glsl", "shaders/PBR.glsl" };
	const char* Material::renderModes[] = { "Opaque", "Transparent" };
    
	Material::Material(const char* path) :
		m_mode(MODE_OPAQUE),
		m_chosenShader(0),
		m_shader(nullptr),
		m_name(path),
		m_uniformData(nullptr),
		m_uniformDataSize(0),
		b_dirty(false)
    {
		Deserialize(FileSystem::ReadStringStream(FileSystem::Path(m_name).c_str()));
		if(m_uniformData == nullptr) InitializeUniformsFromShader();
		AssignTextureSlot();
    }

	Material::Material(Shader * shader) :
		m_mode(MODE_OPAQUE),
		m_chosenShader(0),
		m_shader(shader),
		m_name("RAW_MATERIAL"),
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
		// reserve texture array
		int numOfTextures = 0;
		const std::vector<UniformDeclaration>& declarations = m_shader->GetUniformDeclarations();
		for (auto decl : declarations) if (decl.Type == UniformDeclaration::SAMPLER2D) numOfTextures++;
		m_textures.resize(numOfTextures);

		// assign each texture into its unique slot
		size_t offset = 0;
		uint slot = 0;
		for (auto decl : declarations) {
			if (decl.Type == UniformDeclaration::SAMPLER2D) {
				memcpy(m_uniformData + offset, &slot, sizeof(uint));
				slot++;
			}
			offset += decl.Size();
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

	void Material::OnImGuiRender()
	{
		ImVec2 previewSize(24, 24);
		Texture2D* notFound = TextureLibrary::Placeholder();
        static std::string selectedTexture;

		// Shader
		int prevChosenShader = m_chosenShader;
		ImGui::Combo("Shader", &m_chosenShader, shaders, IM_ARRAYSIZE(shaders));
		if (prevChosenShader != m_chosenShader) {
			ChangeShaderType();
			UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_chosenShader, prevChosenShader, m_chosenShader, "Changed shader mode to " + std::string(shaders[m_chosenShader]) + " for " + EditorLayer::s_selectedGameObject->GetName(), &Material::ChangeShaderType));
		}

		// Rendering Mode
		ImGui::PushItemWidth(110);
		RenderingMode prevMode = m_mode;
		ImGui::Combo("Rendering Mode", (int*)&m_mode, renderModes, IM_ARRAYSIZE(renderModes));
		if (prevMode != m_mode) {
			UndoSystem::GetInstance()->Push(new PropertyAssignmentCommand(this, &m_mode, prevMode, m_mode, "Changed material rendering mode to " + std::string(renderModes[m_mode]) + " for " + EditorLayer::s_selectedGameObject->GetName(), &Material::SetDirty));
			SetDirty();
		}
		ImGui::PopItemWidth();

		// Uniforms
		auto declaration = m_shader->GetUniformDeclarations();
		size_t offset = 0;
		for (auto decl : declaration) {
			void* data = m_uniformData + offset;
			std::string str_id = decl.Name + "##" + m_name;
			ImGui::PushID(str_id.c_str());
			switch (decl.Type) 
			{
				case UniformDeclaration::BOOL:
					ImGui::Checkbox(str_id.c_str(), (bool*)data); break;
				case UniformDeclaration::INT:
					ImGui::SliderInt(str_id.c_str(), (int*)data, (int)decl.Min, (int)decl.Max); break;
				case UniformDeclaration::FLOAT:
					ImGui::SliderFloat(str_id.c_str(), (float*)data, decl.Min, decl.Max); break;
				case UniformDeclaration::VEC3:
					ImGui::ColorEdit3(str_id.c_str(), (float*)data); break;
				case UniformDeclaration::VEC4:
					ImGui::ColorEdit4(str_id.c_str(), (float*)data); break;
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

		// Save configurations
		bool disableButton = !b_dirty;
		if (disableButton) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button("Save Configurations")) {
			if (m_name == "RAW_MATERIAL") {
				ImGui::OpenPopup("New Material");
			}
			else {
				SaveConfiguration();
			}
		}
		if (disableButton) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		static char rename[128] = "";
		if (ImGui::BeginPopupModal("New Material", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Input a name:");
			ImGui::InputText("", rename, IM_ARRAYSIZE(rename));
			std::string validName;
			// Prompt error message (if any)
			if (rename[0] == '\0') {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
				ImGui::Text("Material name cannot be empty.");
				ImGui::PopStyleColor();
			}
			else {
				validName = "materials/" + StringOps::GetValidFilename(std::string(rename)) + ".mat";
				ImGui::Text("Material will be named as: %s", validName.c_str());
			}
			// Confirm and cancel
			if (ImGui::Button(" OK ")) {
				m_name = validName;
				SaveConfiguration();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				m_name = "RAW_MATERIAL";
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
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

	void Material::SetRawTexture2D(int slot, Texture2D * data)
	{
		if (slot < 0 || slot > m_textures.size())
			return;
		m_textures[slot] = data;
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
		std::stringstream ss = Serialize();
		FileSystem::WriteStringStream(FileSystem::Path(m_name).c_str(), ss);

		b_dirty = false;
	}

	void Material::ChangeShaderType() {
		m_shader = ShaderLibrary::Use(shaderPath[m_chosenShader]);
		ResizeUniformBuffer(m_shader->GetUniformBufferSize());
		AssignTextureSlot();
		InitializeUniformsFromShader();
		b_dirty = true;
	}

	std::stringstream Material::Serialize()
	{
		std::stringstream ss;
		{
			cereal::BinaryOutputArchive oachive(ss);
			oachive(*this);
		}
		return ss;
	}

	void Material::Deserialize(std::stringstream ss)
	{
		try {
			cereal::BinaryInputArchive iarchive(ss);
			iarchive(*this);
		}
		catch (std::exception e) {
			m_shader = ShaderLibrary::Use("shaders/Phong.glsl");
			m_uniformDataSize = m_shader->GetUniformBufferSize();
			if (m_uniformData == nullptr) m_uniformData = new byte[m_uniformDataSize];
			WARN("Couldn't load material {}, setting to default. Exception: {}", m_name, e.what());
		};
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

	Material * MaterialLibrary::UseDefault()
	{
		static Material* defaultMaterial = MaterialLibrary::UseShader("shaders/Phong.glsl");
		return defaultMaterial;
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
