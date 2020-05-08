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
		void Serialize(cereal::JSONOutputArchive& oarchive);
		void Deserialize(cereal::JSONInputArchive& iarchive);
		inline TextureCube* Get() const { return m_cubemap; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			std::vector<std::string> faces;
			for (int i = 0; i < 6; ++i) faces.push_back(FileSystem::PathUnderRes(m_faces[i]));
			ar(faces);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			std::vector<std::string> faces;
			ar(faces);
			for (int i = 0; i < 6; ++i) m_faces[i] = FileSystem::Path(faces[i]);
		}
	};

}