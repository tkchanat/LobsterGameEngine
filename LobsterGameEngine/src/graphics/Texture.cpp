#include "pch.h"
#include "Texture.h"
#include "graphics/Shader.h"
#include "graphics/VertexArray.h"
#include "graphics/meshes/MeshFactory.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

namespace Lobster
{
	
	// =======================================================
	// Texture2D
	// =======================================================
	Texture2D::Texture2D(const char* path) :
		m_id(0), m_name(path)
	{
		//	Generate texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		b_loadSuccess = Load();
		if (!b_loadSuccess)
		{
			WARN("Couldn't load texture {}", m_name);
		}
	}

	Texture2D::Texture2D(byte* buffer, const char* id, int w, int h) : m_id(0), m_name(id)
	{
		//	Generate texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_id);
	}

	void Texture2D::SetRaw(byte * data, uint size)
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	bool Texture2D::Load()
	{
		//	Use stb load image
		byte* data = stbi_load(FileSystem::Path(m_name).c_str(), &m_width, &m_height, &m_channelCount, 4);
		if (data == nullptr)
		{
			return false;
		}
		SetRaw(data, m_width * m_height * m_channelCount);

		//	Free the image
		stbi_image_free(data);
		return true;
	}

	// =======================================================
	// TextureCubeMap
	// =======================================================

	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	TextureCube::TextureCube()
	{
		// Generate and bind texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		// Set sampler parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// Frame buffers
		glGenFramebuffers(1, &m_captureFrameBuffer);
		glGenRenderbuffers(1, &m_captureRenderBuffer);
	}

	TextureCube::~TextureCube()
	{
		glDeleteFramebuffers(1, &m_captureFrameBuffer);
		glDeleteRenderbuffers(1, &m_captureRenderBuffer);
		glDeleteTextures(1, &m_id);
		glDeleteTextures(1, &m_irradianceId);
		glDeleteTextures(1, &m_prefilterId);
		glDeleteTextures(1, &m_brdfId);
	}

	void TextureCube::Set(const char * right, const char * left, const char * up, const char * down, const char * back, const char * front)
	{
		if (!(right && left && up && down && back && front)) {
			WARN("One of the faces is missing, abort setting cubemap...");
			return;
		}
		std::vector<std::string> faces = { right, left, up, down, back, front };
		b_loadSuccess = Load(faces);
		if (!b_loadSuccess) {
			WARN("Couldn't load cube map");
		}
	}

	bool TextureCube::Load(const std::vector<std::string>& faces)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		int max_width = 1;
		int max_height = 1;
		struct ImageDesc {
			int width, height;
			int channels;
			unsigned char* data;
		} desc[6];
		// Load individual 2D texture	
		for (int i = 0; i < 6; ++i) {
			desc[i].data = stbi_load(faces[i].c_str(), &desc[i].width, &desc[i].height, &desc[i].channels, 3);
			if (desc[i].width > max_width) max_width = desc[i].width;
			if (desc[i].height > max_height) max_height = desc[i].height;
		}
		m_width = max_width;
		m_height = max_height;
		// Resize images to fix max_width and max_height
		for (int i = 0; i < 6; ++i) {
			if (desc[i].width != max_width || desc[i].height != max_height) {
				unsigned char* input_pixels = desc[i].data;
				unsigned char* output_pixels = new unsigned char[max_width * max_height * 3];
				int result = stbir_resize_uint8(input_pixels, desc[i].width, desc[i].height, 0, output_pixels, max_width, m_height, 0, 3);
				if (result) {
					stbi_image_free(input_pixels);
					desc[i].data = output_pixels;
				}
			}
		}
		// Set images to GPU
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, max_width, max_height, 0, GL_RGB, GL_UNSIGNED_BYTE, desc[i].data);
		}
		// Memory cleaning
		bool failed = false;
		for (int i = 0; i < 6; ++i) {
			failed |= desc[i].data == nullptr;
			stbi_image_free(desc[i].data);
			desc[i].data = nullptr;
		}
		if (failed) {
			WARN("Cubemap texture failed to load");
			return false;
		}

		// Generate PBR resources
		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		GenerateIrradianceMap();
		GeneratePrefilterMap();
		GenerateBRDFMap();
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
		return true;
	}

	void TextureCube::GenerateIrradianceMap()
	{
		// Create irradiance map resources
		glGenTextures(1, &m_irradianceId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceId);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
		
		// Perform convolution on cube map
		Shader* irradianceShader = ShaderLibrary::Use("shaders/IrradianceConvolution.glsl");
		irradianceShader->Bind();
		irradianceShader->SetUniform("environmentMap", 0);
		irradianceShader->SetUniform("captureProjection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		VertexArray* cube = MeshFactory::Cube();
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->SetUniform("captureView", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube->Draw();
		}
		if(cube) delete cube;
		cube = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void TextureCube::GeneratePrefilterMap()
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glGenTextures(1, &m_prefilterId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterId);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		Shader* prefilterShader = ShaderLibrary::Use("shaders/PrefilterConvolution.glsl");
		prefilterShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		prefilterShader->SetUniform("environmentMap", 0);
		prefilterShader->SetUniform("captureProjection", captureProjection);

		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		unsigned int maxMipLevels = 5;
		VertexArray* cube = MeshFactory::Cube();
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// resize framebuffer according to mip-level size.
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, m_captureRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->SetUniform("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				prefilterShader->SetUniform("captureView", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterId, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				cube->Draw();
			}
		}
		if (cube) delete cube;
		cube = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void TextureCube::GenerateBRDFMap()
	{
		// pre-allocate enough memory for the LUT texture.
		glGenTextures(1, &m_brdfId);
		glBindTexture(GL_TEXTURE_2D, m_brdfId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VertexArray* quad = MeshFactory::Plane();
		Shader* brdfShader = ShaderLibrary::Use("shaders/BRDFConvolution.glsl");
		brdfShader->Bind();
		quad->Draw();
		if (quad) delete quad;
		quad = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// =======================================================
	// TextureLibrary
	// =======================================================
	TextureLibrary* TextureLibrary::s_instance = nullptr;

	void TextureLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("TextureLibrary already existed!");
		}
		s_instance = new TextureLibrary();
	}

	void TextureLibrary::Clear()
	{
		for (std::pair<std::string, Texture2D*> element : s_instance->m_textures) {
			if(element.second)
				delete element.second;
			element.second = nullptr;
		}
		s_instance->m_textures.clear();
	}

	Texture2D* TextureLibrary::Use(const char* path)
	{
		if (s_instance->m_textures.find(path) == s_instance->m_textures.end()) {
			Texture2D* newTexture = new Texture2D(path);
			s_instance->m_textures[path] = newTexture;
			return newTexture;
		}
		return s_instance->m_textures[path];
	}

	Texture2D* TextureLibrary::Use(const char* id, byte* buffer, int w, int h) {
		char name[32];
		sprintf(name, "textsprite-%s", id);
		// search for texture with given id
		Texture2D* texFound = nullptr;
		if (s_instance->m_textures.find(name) != s_instance->m_textures.end()) {
			texFound = s_instance->m_textures[std::string(name)];
		}
		// search for texture only: return texture found
		if (!buffer || !w || !h) {
			return texFound;
		}
		// edit texture: delete and return a new one
		else if (texFound) {			
			delete texFound;
			texFound = s_instance->m_textures[std::string(name)] = new Texture2D(buffer, name, w, h);
		}
		// add texture: create a new one and push into library
		else {
			texFound = new Texture2D(buffer, name, w, h);
			s_instance->m_textures[std::string(name)] = texFound;
		}			
		return texFound;
	}

}
