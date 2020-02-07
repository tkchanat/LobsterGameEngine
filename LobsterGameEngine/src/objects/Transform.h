#pragma once

namespace Lobster
{
    
	//    This class is for defining a spatial description of an object.
	//    Any objects composite with this class can do all kinds of affine transform, and has a homogeneous coordinate.
	class Transform
	{
	public:
		glm::vec3 WorldPosition;
		glm::quat WorldRotation;
		glm::vec3 WorldEulerAngles;
        glm::vec3 LocalEulerAngles;
		glm::vec3 LocalPosition;
        glm::quat LocalRotation;		
		glm::vec3 LocalScale;
		float OverallScale;
	private:
		glm::mat4 m_matrix;
	public:
		Transform();
		~Transform() = default;
		void UpdateMatrix();
		void Translate(float dx, float dy, float dz);
		void RotateEuler(float degree, glm::vec3 axis);
		void RotateAround(float degree, glm::vec3 axis, glm::vec3 point);
		void LookAt(glm::vec3 at); 
		inline glm::mat4 GetMatrix() const { return m_matrix; }
		inline glm::mat3 GetBasis() const { return glm::mat3(Right(), Up(), Forward()); }
		inline glm::vec3 Right() const { return LocalRotation * glm::vec3(1, 0, 0); }
		inline glm::vec3 Up() const { return LocalRotation * glm::vec3(0, 1, 0); }
		inline glm::vec3 Forward() const { return LocalRotation * glm::vec3(0, 0, 1); }
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(WorldPosition);
			ar(WorldRotation);
			ar(WorldEulerAngles);
			ar(LocalEulerAngles);
			ar(LocalPosition);
			ar(LocalRotation);
			ar(LocalScale);
			ar(OverallScale);
		}
	};
    
}
