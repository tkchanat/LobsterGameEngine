#pragma once

namespace Lobster
{

	enum class Formats : int
	{
		AlphaFormat = GL_ALPHA,
		RedFormat = GL_RED,
		RedIntFormat = GL_RED_INTEGER,
		RGFormat = GL_RG,
		RGIntFormat = GL_RG_INTEGER,
		RGBFormat = GL_RGB,
		RGB16FFormat = GL_RGB16F,
		RGBIntFormat = GL_RGB_INTEGER,
		RGBAFormat = GL_RGBA,
		RGBA16FFormat = GL_RGBA16F,
		RGBAIntFormat = GL_RGBA_INTEGER,
		DepthComponentFormat = GL_DEPTH_COMPONENT
	};

	enum class Types : int
	{
		UnsignedByteType = GL_UNSIGNED_BYTE,
		ByteType = GL_BYTE,
		ShortType = GL_SHORT,
		UnsignedShortType = GL_UNSIGNED_SHORT,
		IntType = GL_INT,
		UnsignedIntType = GL_UNSIGNED_INT,
		FloatType = GL_FLOAT,
		HalfFloatType = GL_HALF_FLOAT
	};

	enum class MinMagFilters : int
	{
		NearestFilter = GL_NEAREST,
		LinearFilter = GL_LINEAR
	};

	enum class WrappingModes : int
	{
		RepeatWrapping = GL_REPEAT,
		MirroredRepeatWrapping = GL_MIRRORED_REPEAT,
		ClampToEdgeWrapping = GL_CLAMP_TO_EDGE,
		ClampToBorderWrapping = GL_CLAMP_TO_BORDER
	};

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
		std::string m_name;
	public:
		virtual ~Texture2D() override;
		void SetRaw(byte* data, uint size);
		inline virtual void* Get() const override { return (void*)(intptr_t)m_id; }
		inline std::string GetPath() const { return FileSystem::Path(m_name); }
		inline std::string GetName() const { return FileSystem::PathUnderRes(m_name); }
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
		// irradiance & prefilter map
		uint m_irradianceId;
		uint m_prefilterId;
		uint m_brdfId;
		uint m_captureFrameBuffer;
		uint m_captureRenderBuffer;
	public:
		TextureCube();
		virtual ~TextureCube() override;
		void Set(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front);
		inline virtual void* Get() const override { return(void*)(intptr_t)m_id; }
		inline void* GetIrradiance() const { return (void*)(intptr_t)m_irradianceId; }
		inline void* GetPrefilter() const { return (void*)(intptr_t)m_prefilterId; }
		inline void* GetBRDF() const { return (void*)(intptr_t)m_brdfId; }
	protected:
		bool Load(const std::vector<std::string>& faces);
		void GenerateIrradianceMap();
		void GeneratePrefilterMap();
		void GenerateBRDFMap();
	};

	class TextureLibrary
	{
		friend class Renderer;
	private:
		std::unordered_map<std::string, Texture2D*> m_textures;
		static TextureLibrary* s_instance;
	public:
		static void Initialize();
		static void Clear();
		static Texture2D* Use(const char* path);
		// search, edit or add texture to the corresponding id
		static Texture2D* Use(const char* id, byte* buffer, int w, int h);
		inline static Texture2D* Placeholder() { return TextureLibrary::Use("textures/image_not_found.png"); } // only use this in debug mode
	};

}
