#pragma once

#include <common/Matrix3.h>
#include <common/Vector3.h>


namespace hephaestus
{
class Matrix4;

class Camera
{
public:

	const Matrix3& GetRotation() const { return m_rotation; }
	const Vector3& GetTranslation() const { return m_translation; }

	void Set(const Matrix3& rotation, const Vector3& translation)
	{
		m_rotation = rotation;
		m_translation = translation;
	}

	enum MoveDirection : int
	{
		eMOVE_NULL = 0,
		eMOVE_FORWARD = 1,
		eMOVE_BACKWARDS = 2,
		eMOVE_LEFT = 3,
		eMOVE_RIGHT = 4,

		eMOVE_MAX // unused
	};

	void MoveFreeForm(MoveDirection direction, float speed = 1.f);
	void RotateFromMouseDeltaFreeForm(float dx, float dy, float speed = 1.f);

	void SetLookAt(const Vector3& position, const Vector3& target = Vector3::ZERO);
	void MoveLookAt(MoveDirection direction, float speed = 1.f, const Vector3& target = Vector3::ZERO);
	void RotateFromMouseDeltaLookAt(float dx, float dy, float speed = 1.f, const Vector3& target = Vector3::ZERO);

	void GetViewRenderMatrix(Matrix4& view) const;

private:
	Matrix3 m_rotation = Matrix3::IDENTITY;
	Vector3 m_translation = Vector3::ZERO;
};

}