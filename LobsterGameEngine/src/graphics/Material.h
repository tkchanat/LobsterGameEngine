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
		std::vector<std::string> _textureNames;
	public:
		virtual ~Material();
		void OnImGuiRender();
		void SetUniforms();
		void SaveConfiguration();
		inline std::string GetName() const { return m_name; }
		inline std::string GetPath() const { return FileSystem::Path(m_name); }
		inline Shader* GetShader() const { return m_shader; }
		inline RenderingMode GetRenderingMode() const { return m_mode; }
	public:
		template <class Archive>
		void save(Archive & ar) const
		{
			ar(m_shader->GetName()); // Shader name
			ar(_textureNames); // Texture maps
			ar(m_mode);
			ar(m_uniformDataSize);
			ar(cereal::binary_data(m_uniformData, m_uniformDataSize)); // Uniform data
		}
		template <class Archive>
		void load(Archive & ar)
		{
			// shader
			std::string shaderName;
			ar(shaderName);
			m_shader = ShaderLibrary::Use(shaderName.c_str());
			// textures
			ar(_textureNames);
			// rendering mode
			ar(reinterpret_cast<RenderingMode>(m_mode));
			ar(m_uniformDataSize);
			m_uniformData = new byte[m_uniformDataSize];
			ar(cereal::binary_data(m_uniformData, m_uniformDataSize)); // Uniform data
		}
	private:
        Material(const char* path);
		Material(Shader* shader);
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
		static void ResizeUniformBuffer(Shader* shader);
	};
    
}
