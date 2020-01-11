#pragma once
#include "graphics/Shader.h"
#include "graphics/Texture.h"

namespace Lobster
{

	class Shader;
	class Texture2D;

	enum RenderingMode : uint
	{
		MODE_OPAQUE,
		MODE_TRANSPARENT
	};
    
	//	This class defines what are the material properties and the shader used. (To be implemented)
	//	e.g. ambient, diffuse, specular, roughness, reflection, refraction, etc.
	class Material
	{
		friend class MaterialLibrary;
	protected:
		std::string m_name;
		RenderingMode m_mode;
		Shader* m_shader;
		std::vector<Texture2D*> m_textures;
		byte* m_uniformData;
		size_t m_uniformDataSize;
		bool b_dirty;
	public:
		virtual ~Material();
		void OnImGuiRender();
		void SetRawUniform(const char* name, void* data);
		void SetUniforms();
		void SaveConfiguration();
		std::stringstream Serialize();
		void Deserialize(std::stringstream ss);
		inline std::string GetName() const { return m_name; }
		inline std::string GetPath() const { return FileSystem::Path(m_name); }
		inline Shader* GetShader() const { return m_shader; }
		inline RenderingMode GetRenderingMode() const { return m_mode; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			std::string shaderName = m_shader->GetName();
			std::vector<std::string> textureNames;
			for (int i = 0; i < m_textures.size(); ++i) 
				textureNames.push_back(m_textures[i] == nullptr ? "" : m_textures[i]->GetName());
			
			ar(shaderName); // Shader name
			ar(textureNames); // Texture maps
			ar(m_mode);
			ar(m_uniformDataSize);
			ar(cereal::binary_data(m_uniformData, m_uniformDataSize)); // Uniform data
		}
		template <class Archive>
		void load(Archive & ar)
		{
			std::string shaderName;
			std::vector<std::string> textureNames;
			ar(shaderName);
			ar(textureNames);

			m_shader = ShaderLibrary::Use(shaderName.c_str());
			m_textures.resize(textureNames.size());
			for (int i = 0; i < m_textures.size(); ++i)
				m_textures[i] = textureNames[i].empty() ? nullptr : TextureLibrary::Use(textureNames[i].c_str());
			
			ar(reinterpret_cast<RenderingMode>(m_mode));
			ar(m_uniformDataSize);
			m_uniformData = new byte[m_uniformDataSize];
			ar(cereal::binary_data(m_uniformData, m_uniformDataSize)); // Uniform data
		}
	private:
        Material(const char* path);
		Material(Shader* shader);
		void InitializeUniformsFromShader();
		void AssignTextureSlot();
		void ResizeUniformBuffer(size_t newSize);
	};

	class MaterialLibrary
	{
	private:
		std::vector<Material*> m_materials;
		static MaterialLibrary* s_instance;
	public:
		static void Initialize();
		static Material* Use(const char* path);
		static Material* UseShader(const char* shaderPath);
		static Material* UseDefault();
		static void ResizeUniformBuffer(Shader* shader);
	};
    
}
