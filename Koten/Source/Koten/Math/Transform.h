#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>



namespace KTN::Math
{
	struct KTN_API WorldMatrix
	{
		WorldMatrix() = default;
		WorldMatrix(const WorldMatrix&) = default;
		WorldMatrix(const glm::mat4& p_Matrix);
		~WorldMatrix() = default;

		glm::mat4 Matrix{ 1.0f };

		glm::vec3 Translation{ 0.0f };
		glm::vec3 Rotation{ 0.0f }; // In Degrees
		glm::vec3 Scale{ 1.0f };
		bool Initialized = false;
	};

	class KTN_API Transform
	{
		public:
			Transform();
			Transform(const Transform&) = default;
			Transform(const glm::mat4& p_LocalMatrix, const glm::mat4& p_WorldMatrix = glm::mat4(1.0f));
			// p_LocalRotation In Degrees
			Transform(const glm::vec3& p_LocalTranslation, const glm::vec3& p_LocalScale = { 1.0f, 1.0f, 1.0f }, const glm::vec3& p_LocalRotation = { 0.0f, 0.0f, 0.0f });
			~Transform() = default;

			void SetWorldMatrix(const glm::mat4& p_Matrix);
			void SetLocalMatrix(const glm::mat4& p_Matrix);

			void SetLocalTranslation(const glm::vec3& p_Vector);
			// In Degrees
			void SetLocalRotation(const glm::vec3& p_Vector);
			void SetLocalScale(const glm::vec3& p_Vector);

			const glm::vec3& GetLocalTranslation() const;
			const glm::vec3& GetLocalRotation() const;
			const glm::vec3& GetLocalScale() const;

			const glm::vec3& GetWorldTranslation() const;
			const glm::vec3& GetWorldRotation() const;
			const glm::vec3& GetWorldScale() const;

			glm::quat GetWorldOrientation() const;
			glm::quat GetLocalOrientation() const;

			glm::vec3 GetWorldUpDirection() const { return GetOrientationDir(GetWorldOrientation(), { 0.0f, 1.0f, 0.0f }); }
			glm::vec3 GetWorldRightDirection() const { return GetOrientationDir(GetWorldOrientation(), { 1.0f, 0.0f, 0.0f }); }
			glm::vec3 GetWorldFrontDirection() const { return GetOrientationDir(GetWorldOrientation(), { 0.0f, 0.0f, -1.0f }); }

			glm::vec3 GetLocalUpDirection() const { return GetOrientationDir(GetLocalOrientation(), { 0.0f, 1.0f, 0.0f }); }
			glm::vec3 GetLocalRightDirection() const { return GetOrientationDir(GetLocalOrientation(), { 1.0f, 0.0f, 0.0f }); }
			glm::vec3 GetLocalFrontDirection() const { return GetOrientationDir(GetLocalOrientation(), { 0.0f, 0.0f, -1.0f }); }

			glm::mat4 GetWorldMatrix() const;
			glm::mat4 GetLocalMatrix() const;

			static bool Decompose(const glm::mat4& p_Matrix, glm::vec3& p_Translation, glm::vec3& p_Scale, glm::vec3& p_Rotation);

		private:
			glm::vec3 GetOrientationDir(const glm::quat& p_Orientation, const glm::vec3& p_Dir) const
			{
				glm::vec3 dir = p_Dir;
				dir = p_Orientation * dir;
				return dir;
			}

		private:
			WorldMatrix m_WorldMatrix;

			glm::vec3 m_LocalTranslation{ 0.0f, 0.0f, 0.0f };
			glm::vec3 m_LocalRotation{ 0.0f, 0.0f, 0.0f };
			glm::vec3 m_LocalScale{ 1.0f, 1.0f, 1.0f };
	};


} // namespace KTN::Math