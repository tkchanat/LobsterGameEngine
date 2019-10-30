#include "pch.h"
#include "Texture.h"

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
	TextureCube::TextureCube(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front)
	{
		//	Generate and bind texture
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
		//  Set sampler parameters
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
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_id);
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
