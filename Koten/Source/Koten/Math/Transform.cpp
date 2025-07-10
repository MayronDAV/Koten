#include "ktnpch.h"
#include "Transform.h"

// lib
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>



namespace KTN::Math
{
	static glm::mat4 Translate(const glm::vec3& p_Vector)
	{
		return glm::translate(glm::mat4(1.0f), p_Vector);
	}

	static glm::mat4 Scale(const glm::vec3& p_Vector)
	{
		return glm::scale(glm::mat4(1.0f), p_Vector);
	}

	static glm::mat4 Rotate(const glm::vec3& p_Vector)
	{
		return glm::toMat4(glm::quat(p_Vector));
	}

	Transform::Transform()
	{
	}

	Transform::Transform(const glm::mat4& p_LocalMatrix, const glm::mat4& p_WorldMatrix)
	{
		SetLocalMatrix(p_LocalMatrix);
		SetWorldMatrix(p_WorldMatrix);
	}

	Transform::Transform(const glm::vec3& p_LocalTranslation, const glm::vec3& p_LocalScale, const glm::vec3& p_LocalRotation)
		: m_LocalTranslation(p_LocalTranslation),
		m_LocalScale(p_LocalScale),
		m_LocalRotation(p_LocalRotation)
	{
	}

	void Transform::SetWorldMatrix(const glm::mat4& p_Matrix)
	{
		m_WorldMatrix = WorldMatrix(p_Matrix * GetLocalMatrix());
	}

	void Transform::SetLocalMatrix(const glm::mat4& p_Matrix)
	{
		if (!Decompose(p_Matrix, m_LocalTranslation, m_LocalScale, m_LocalRotation))
		{
			KTN_WARN("Failed to decompose local matrix!")
		}
	}

	void Transform::SetLocalTranslation(const glm::vec3& p_Vector)
	{
		m_LocalTranslation = p_Vector;
	}

	void Transform::SetLocalScale(const glm::vec3& p_Vector)
	{
		m_LocalScale = p_Vector;
	}

	void Transform::SetLocalRotation(const glm::vec3& p_Vector)
	{
		m_LocalRotation = p_Vector;
	}

	const glm::vec3& Transform::GetLocalTranslation() const
	{
		return m_LocalTranslation;
	}

	const glm::vec3& Transform::GetLocalRotation() const
	{
		return m_LocalRotation;
	}

	const glm::vec3& Transform::GetLocalScale() const
	{
		return m_LocalScale;
	}

	const glm::vec3& Transform::GetWorldTranslation() const
	{
		if (m_WorldMatrix.Initialized)
			return m_WorldMatrix.Translation;

		return GetLocalTranslation();
	}

	const glm::vec3& Transform::GetWorldRotation() const
	{
		if (m_WorldMatrix.Initialized)
			return m_WorldMatrix.Rotation;

		return GetLocalRotation();
	}

	const glm::vec3& Transform::GetWorldScale() const
	{
		if (m_WorldMatrix.Initialized)
			return m_WorldMatrix.Scale;

		return GetLocalScale();
	}

	glm::quat Transform::GetWorldOrientation() const
	{
		return glm::quat(m_WorldMatrix.Rotation);
	}

	glm::quat Transform::GetLocalOrientation() const
	{
		return glm::quat(m_LocalRotation);
	}

	glm::mat4 Transform::GetWorldMatrix() const
	{
		if (m_WorldMatrix.Initialized)
			return m_WorldMatrix.Matrix;

		return GetLocalMatrix();
	}

	glm::mat4 Transform::GetLocalMatrix() const
	{
		return Translate(m_LocalTranslation) * Rotate(m_LocalRotation) * Scale(m_LocalScale);
	}

	bool Transform::Decompose(const glm::mat4& p_Matrix, glm::vec3& p_Translate, glm::vec3& p_Scale, glm::vec3& p_Rotation)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(p_Matrix);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		p_Translate = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3]{}, Pdum3{};

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		p_Scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		p_Scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		p_Scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.

		p_Rotation.y = asin(-Row[0][2]);
		if (cos(p_Rotation.y) != 0)
		{
			p_Rotation.x = atan2(Row[1][2], Row[2][2]);
			p_Rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else
		{
			p_Rotation.x = atan2(-Row[2][0], Row[1][1]);
			p_Rotation.z = 0;
		}


		return true;
	}

	WorldMatrix::WorldMatrix(const glm::mat4& p_Matrix)
	{
		Matrix = p_Matrix;
		if (!Transform::Decompose(Matrix, Translation, Scale, Rotation))
		{
			KTN_WARN("Failed to decompose world matrix!")
				return;
		}

		Initialized = true;
	}

} // namespace KTN::Math
