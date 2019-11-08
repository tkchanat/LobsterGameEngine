#pragma once
#include "graphics/UniformBuffer.h"

namespace Lobster
{

	class Shader;
	class Texture2D;
    
	//	This class defines what are the material properties and the shader used. (To be implemented)
	//	e.g. ambient, diffuse, specular, roughness, reflection, refraction, etc.
    class Material
    {
	protected:
		//	One material must have one shader. If not, either not render it or render it in error color.
        Shader* m_shader;
		std::vector<std::pair<std::string, Texture2D*>> m_textureUnits;
		std::vector<std::pair<std::string, UniformBufferData*>> m_uniformBufferData;
		std::string m_name;
		std::string m_path;
		JsonFile m_json;
        ImGui::FileBrowser m_fileBrowser;
    public:
        Material(const char* path);
        virtual ~Material();
		void OnImGuiRender();
		void SaveConfiguration();
		void SetTextureUnit(const char* name, const char* texturePath);
		inline bool Exist() const { return m_json.getFileExist(); }
		inline std::string GetPath() const { return m_path; }
        inline Shader* GetShader() const { return m_shader; }
		inline Texture2D* GetTextureUnit(int index) const { return index < m_textureUnits.size() ? m_textureUnits[index].second : nullptr; }
		inline UniformBufferData* GetUniformBufferData(int index) const { return index < m_uniformBufferData.size() ? m_uniformBufferData[index].second : nullptr; }
    };
    
}
