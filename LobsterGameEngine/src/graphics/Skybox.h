#pragma once

namespace Lobster
{

	class TextureCube;

	class Skybox
	{
	private:
		std::string m_faces[6];
		TextureCube* m_cubemap;
	public:
		Skybox(const char* right, const char* left, const char* up, const char* down, const char* back, const char* front);
		~Skybox();
		void OnImGuiRender(bool* p_open);
		inline TextureCube* Get() const { return m_cubemap; }
	};

}