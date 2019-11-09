#pragma once
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/UniformBuffer.h"

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
		RenderingMode m_mode;
        Shader* m_shader;
		std::vector<std::pair<std::string, Texture2D*>> m_textureUnits;
		std::vector<std::pair<std::string, UniformBufferData*>> m_uniformBufferData;
		std::string m_name;
		std::string m_path;
		JsonFile m_json;
		bool b_dirty;
    public:
        virtual ~Material();
		void OnImGuiRender();
		void SaveConfiguration();
		void SetTextureUnit(const char* name, const char* texturePath);
		inline bool Exist() const { return m_json.getFileExist(); }
		inline std::string GetName() const { return m_name; }
		inline std::string GetPath() const { return m_path; }
        inline Shader* GetShader() const { return m_shader; }
		inline RenderingMode GetRenderingMode() const { return m_mode; }
		inline Texture2D* GetTextureUnit(int index) const { return index < m_textureUnits.size() ? m_textureUnits[index].second : nullptr; }
		inline UniformBufferData* GetUniformBufferData(int index) const { return index < m_uniformBufferData.size() ? m_uniformBufferData[index].second : nullptr; }
	private:
        Material(const char* path);
	};

	class MaterialLibrary
	{
	private:
		std::vector<Material*> m_materials;
		static MaterialLibrary* s_instance;
	public:
		static void Initialize();
		static Material* Use(const char* path);
	};
    
}
