#pragma once

namespace Lobster
{

	class Texture
	{
	protected:
		int m_width, m_height;
		int m_channelCount;
		bool b_loadSuccess;
	public:
		virtual ~Texture() {}
		virtual void* Get() const = 0;
	protected:
		Texture() : m_width(1), m_height(1), m_channelCount(4), b_loadSuccess(false) {}
	};

	class Texture2D : Texture
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
		bool Load();
	};

	class TextureCube : public Texture
	{
	protected:
		uint m_id;
		std::string m_rightPath;
		std::string m_leftPath;
		std::string m_upPath;
		std::string m_downPath;
		std::string m_backPath;
		std::string m_frontPath;
	public:
		virtual ~TextureCube() override;
		inline virtual void* Get() const override { return(void*)(intptr_t)m_id; }
		explicit TextureCube(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front);
	protected:
		bool Load();
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
		inline static Texture2D* Placeholder() { return s_instance->m_placeholder; } // only use this in debug mode
	};

}
