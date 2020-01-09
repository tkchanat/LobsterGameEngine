#include "pch.h"
#include "Texture.h"
#include "graphics/Shader.h"
#include "graphics/VertexArray.h"
#include "graphics/meshes/MeshFactory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Lobster
{
	
	// =======================================================
	// Texture2D
	// =======================================================
	Texture2D::Texture2D(const char* path) :
		m_id(0),
		m_path(FileSystem::Path(path)),
		m_name(path)
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
			WARN("Couldn't load texture {}", m_path);
		}
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
		byte* data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_channelCount, 4);
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

	VertexArray* TextureCube::s_cube = nullptr;
	VertexArray* TextureCube::s_quad = nullptr;
	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	TextureCube::TextureCube(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front)
	{
		// Initialize cube and quad mesh if either one is not set
		if (!s_cube || !s_quad)
		{
			s_cube = MeshFactory::Cube();
			s_quad = MeshFactory::Plane();
		}

		// Generate and bind texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		// Set sampler parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		m_rightPath = FileSystem::Path(right);
		m_leftPath	= FileSystem::Path(left);
		m_upPath	= FileSystem::Path(up);
		m_downPath	= FileSystem::Path(down);
		m_backPath	= FileSystem::Path(back);
		m_frontPath = FileSystem::Path(front);

		b_loadSuccess = Load();
		if (!b_loadSuccess)
		{
			WARN("Couldn't load cube map");
		}

		GenerateIrradianceMap();
		GeneratePrefilterMap();
		GenerateBRDFMap();
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_id);
		glDeleteTextures(1, &m_irradianceId);
		glDeleteTextures(1, &m_prefilterId);
		glDeleteTextures(1, &m_brdfId);
		glDeleteFramebuffers(1, &m_captureFrameBuffer);
		glDeleteRenderbuffers(1, &m_captureRenderBuffer);
	}

	bool TextureCube::Load()
	{
		//  Specify in OpenGL cube map enum order
		std::vector<std::string> faces = {
			m_rightPath,
			m_leftPath,
			m_upPath,
			m_downPath,
			m_backPath,
			m_frontPath
		};

		//  Load individual 2D texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char *data = stbi_load(faces[i].c_str(), &m_width, &m_height, &m_channelCount, 4);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				WARN("Cubemap texture failed to load at path: {}", faces[i]);
				stbi_image_free(data);
				return false;
			}
		}
		return true;
	}

	void TextureCube::GenerateIrradianceMap()
	{
		// Create irradiance map resources
		glGenFramebuffers(1, &m_captureFrameBuffer);
		glGenRenderbuffers(1, &m_captureRenderBuffer);
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

		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->SetUniform("captureView", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			s_cube->Draw();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
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
		prefilterShader->SetUniform("environmentMap", 0);
		prefilterShader->SetUniform("captureProjection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		unsigned int maxMipLevels = 5;
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
				s_cube->Draw();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
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
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFrameBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfId, 0);

		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		glViewport(0, 0, 512, 512);
		Shader* brdfShader = ShaderLibrary::Use("shaders/BRDFConvolution.glsl");
		brdfShader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s_quad->Draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
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
		s_instance->m_placeholder = TextureLibrary::Use("textures/image_not_found.png");
	}

	Texture2D* TextureLibrary::Use(const char* path)
	{
		for (Texture2D* texture : s_instance->m_textures)
		{
			if (texture->GetName() == path)
			{
				return texture;
			}
		}
		Texture2D* newTexture = new Texture2D(path);
		s_instance->m_textures.push_back(newTexture);
		return newTexture;
	}

}
