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
		byte* m_uniformData;
		size_t m_uniformDataSize;
		std::vector<Texture2D*> m_textures;
		std::string m_name;
		JsonFile m_json;
		bool b_dirty;
    public:
        virtual ~Material();
		void OnImGuiRender();
		void SetUniforms();
		void SaveConfiguration();
		inline bool Exist() const { return m_json.getFileExist(); }
		inline std::string GetName() const { return m_name; }
		inline std::string GetPath() const { return FileSystem::Path(m_name); }
        inline Shader* GetShader() const { return m_shader; }
		inline RenderingMode GetRenderingMode() const { return m_mode; }
	private:
        Material(const char* path);
		void InitializeUniformsFromShader();
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
