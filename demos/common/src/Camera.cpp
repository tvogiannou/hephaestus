#include <common/Camera.h>

#include <common/Matrix4.h>


namespace hephaestus
{

void 
Camera::MoveFreeForm(MoveDirection direction, float speed /*= 1.f*/)
{
    // dispatch table
    static Vector3 s_vec3FromDirection[] =
    {
        Vector3::ZERO,			// eMOVE_NULL
        Vector3::NEG_UNIT_Z,	// eMOVE_FORWARD
        Vector3::UNIT_Z,		// eMOVE_BACKWARDS
        Vector3::NEG_UNIT_X,	// eMOVE_LEFT
        Vector3::UNIT_X,		// eMOVE_RIGHT
    };

    Vector3 movement = s_vec3FromDirection[direction];
    movement.Mul(speed);

    m_translation.Add(m_rotation.MulVec3Right(movement));
}

void 
Camera::RotateFromMouseDeltaFreeForm(float dx, float dy, float speed /*= 1.f*/)
{
    if (dx == 0.0f && dy == 0.0f)
        return;

    dy = -dy; // y axis for mouse coord goes downwards so invert it
    
    float dtheta = 0.01f * speed; // hardcoded step
    
    // form a rotation axis on the xy plane based on how the mouse moved
    Vector3 rotationAxis(Vector3::ZERO);
    {
        if ((dx >= 0 && dy >= 0) || (dx <= 0 && dy <= 0))
        {
            rotationAxis.x = -(float)dy;
            rotationAxis.y = (float)dx;
            dtheta = -dtheta;
        }
        else
        {
            rotationAxis.x = (float)dy;
            rotationAxis.y = -(float)dx;
        }
    }

    if (rotationAxis.Dot() > 1e-4f)
    {
        Matrix3 rotation;
        rotation.SetFromRotationAxis(rotationAxis, dtheta);

        m_rotation.Mul(rotation);
    }
}

void 
Camera::SetLookAt(const Vector3& position, const Vector3& target /*= Vector3::ZERO*/)
{
    m_translation = position;

    Vector3 zAxis = m_translation;
    zAxis.Sub(target);
    zAxis.Normalize();

    Vector3 xAxis = Vector3::Cross(Vector3::UNIT_Y, zAxis);
    xAxis.Normalize();

    Vector3 yAxis = Vector3::Cross(zAxis, xAxis);
    yAxis.Normalize();

    m_rotation = Matrix3(
        xAxis.x, yAxis.x, zAxis.x,
        xAxis.y, yAxis.y, zAxis.y,
        xAxis.z, yAxis.z, zAxis.z);
}

void 
Camera::MoveLookAt(MoveDirection direction, float speed /*= 1.f*/, const Vector3& /*target*/ /*= Vector3::ZERO*/)
{
    // dispatch table
    static Vector3 s_vec3FromDirection[] =
    {
        Vector3::ZERO,			// eMOVE_NULL
        Vector3::NEG_UNIT_Z,	// eMOVE_FORWARD
        Vector3::UNIT_Z,		// eMOVE_BACKWARDS
        Vector3::ZERO,			// eMOVE_LEFT
        Vector3::ZERO,			// eMOVE_RIGHT
    };

    Vector3 movement = s_vec3FromDirection[direction];
    movement.Mul(speed);

    m_translation.Add(m_rotation.MulVec3Right(movement));
}

void 
Camera::RotateFromMouseDeltaLookAt(float dx, float dy, float speed /*= 1.f*/, const Vector3& target /*= Vector3::ZERO*/)
{
    if (dx == 0.0f && dy == 0.0f)
        return;

    float dtheta = 0.01f * speed; // hardcoded step

    dy = -dy; // y axis for mouse coord goes downwards so invert it

    Vector3 upVector(m_rotation.Get(0, 1), m_rotation.Get(1, 1), m_rotation.Get(2, 1));

    Matrix3 rotationX(Matrix3::IDENTITY);
    if (dx != 0)
    {
        rotationX.SetFromRotationAxis(upVector, dx > 0 ? -dtheta : dtheta);
    }

    // position of the camera relative to the target
    Vector3 posToTarget = m_translation;
    posToTarget.Sub(target);

    Matrix3 rotationY(Matrix3::IDENTITY);
    if (dy != 0)
    {
        Vector3 dp = posToTarget;
        dp.Normalize();
        const Vector3 u = Vector3::Cross(dp, upVector);
        rotationY.SetFromRotationAxis(u, dy > 0 ? -dtheta : dtheta);
    }

    // new position relative to the target
    Vector3 newPosition = rotationX.MulVec3Right(rotationY.MulVec3Right(posToTarget));

    // new position to world space
    newPosition.Add(target);

    // compute rotation
    SetLookAt(newPosition, target);
}

void 
Camera::GetViewRenderMatrix(Matrix4& view) const
{
    view.AffineSet();
    view.AffineSetRotation(m_rotation);
    view.AffineSetTranslation(m_translation);
    view.AffineInverse();
}

}
