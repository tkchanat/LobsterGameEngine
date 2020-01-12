#pragma once

namespace Lobster
{

	class VertexArray;

	class Texture
	{
	protected:
		int m_width, m_height;
		int m_channelCount;
		bool b_loadSuccess;
	public:
		virtual ~Texture() {}
		virtual void* Get() const = 0;
		inline int GetWidth() { return m_width; }
		inline int GetHeight() { return m_height; }
	protected:
		Texture() : m_width(1), m_height(1), m_channelCount(4), b_loadSuccess(false) {}
	};

	class Texture2D : public Texture
	{
		friend class TextureLibrary;
	private:
		uint m_id;
		std::string m_path;
		std::string m_name;
	public:
		virtual ~Texture2D() override;
		void SetRaw(byte* data, uint size);
		inline virtual void* Get() const override { return (void*)(intptr_t)m_id; }
		inline std::string GetPath() const { return m_path; }
		inline std::string GetName() const { return m_name; }
	private:
		explicit Texture2D(const char* path);
		// This constructor should only be used to create text sprite
		explicit Texture2D(byte* buffer, const char* id, int w, int h);
		bool Load();
	};

	class TextureCube : public Texture
	{
	protected:
		// cube map
		uint m_id;
		std::string m_rightPath;
		std::string m_leftPath;
		std::string m_upPath;
		std::string m_downPath;
		std::string m_backPath;
		std::string m_frontPath;
		// irradiance & prefilter map
		uint m_irradianceId;
		uint m_prefilterId;
		uint m_brdfId;
		uint m_captureFrameBuffer;
		uint m_captureRenderBuffer;
		// static cube mesh for rendering
		static VertexArray* s_cube;
		static VertexArray* s_quad;
	public:
		virtual ~TextureCube() override;
		inline virtual void* Get() const override { return(void*)(intptr_t)m_id; }
		inline void* GetIrradiance() const { return (void*)(intptr_t)m_irradianceId; }
		inline void* GetPrefilter() const { return (void*)(intptr_t)m_prefilterId; }
		inline void* GetBRDF() const { return (void*)(intptr_t)m_brdfId; }
		inline static VertexArray* GetCube() { return s_cube; }
		explicit TextureCube(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front);
	protected:
		bool Load();
		void GenerateIrradianceMap();
		void GeneratePrefilterMap();
		void GenerateBRDFMap();
	};

	class TextureLibrary
	{
		friend class Renderer;
	private:
		Texture2D* m_placeholder;
		std::vector<Texture2D*> m_textures;
		static TextureLibrary* s_instance;
	public:
		static void Initialize();
		static Texture2D* Use(const char* path);
		// search, edit or add texture to the corresponding id
		static Texture2D* Use(const char* id, byte* buffer, int w, int h);
		inline static Texture2D* Placeholder() { return s_instance->m_placeholder; } // only use this in debug mode
	};

}
