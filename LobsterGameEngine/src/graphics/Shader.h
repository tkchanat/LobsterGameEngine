#pragma once
#include "Uniform.h"

namespace Lobster 
{

	class UniformBuffer;
   
    //  This class is for holding the shader program and playing the role of accessing shader uniforms / constant buffers.
	//	All shaders are searched in the shader folder under StealStepFYP/res/ directory. Please make sure they are there.
	//	Note:	To keep things cleaner, this implementation encourage putting all shader stages code into one .glsl file.
	//			Such that different shader stages are separated by a comment line (e.g. "///VertexShader", or "///FragmentShader")
	//			Otherwise, the shader would not be able to compile.
	//	Learn more about GLSL: https://learnopengl.com/Getting-started/Shaders
    //  TODO:	Make a shader manager class for reducing the number of repeated shader.
    class Shader
    {
		friend class ShaderLibrary;
    private:
        uint m_id;
        uint m_vsId;
        uint m_fsId;
		std::string m_name;
        std::string m_path;
		std::unordered_map<std::string, int> m_uniformLocationMap;
		std::vector<UniformDeclaration> m_uniformDeclarations;
		bool b_compileSuccess;
    public:
		Shader() = delete;
        ~Shader();
        void Bind() const;
        void Unbind() const;
        void Reload();
        inline uint GetID() const { return m_id; }
		inline std::string GetName() const { return m_name; }
		inline std::string GetPath() const { return m_path; }
		inline bool CompileSuccess() const { return b_compileSuccess; }
        //  Shader uniforms
		void SetUniform(const char* name, UniformDeclaration::DataType type, byte* data);
		void SetUniform(const char* name, int data);
		void SetUniform(const char* name, float data);
		void SetUniform(const char* name, const bool& data);
		void SetUniform(const char* name, const glm::vec2& data);
        void SetUniform(const char* name, const glm::vec3& data);
		void SetUniform(const char* name, const glm::vec4& data);
        void SetUniform(const char* name, const glm::mat4& data);
		void SetUniform(const char* name, size_t count, const glm::mat4* data);
		void SetTexture2D(uint slot, void* texture2D);
		void SetTextureCube(uint slot, void* textureCube);
		size_t GetUniformBufferSize() const;
		inline const std::vector<UniformDeclaration>& GetUniformDeclarations() const { return m_uniformDeclarations; }
    private:
        Shader(const char* path);
        bool Compile();
		void ParseUniform();
		void SetBlockBinding(const char* name, int bindingPoint);
    };

	class ShaderLibrary
	{
	private:
		std::vector<Shader*> m_shaders;
		std::vector<std::filesystem::file_time_type> m_shadersLastModified;
		static ShaderLibrary* s_instance;
	public:
		static void Initialize();
		static void LiveReload();
		static Shader* Use(const char* path);
		//static void SetBlockBinding(const char* name, int bindingPoint);
	};

    
}
