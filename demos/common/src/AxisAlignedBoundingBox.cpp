#include <common/AxisAlignedBoundingBox.h>

#include <algorithm> // for std min/max


namespace hephaestus
{

void 
AxisAlignedBoundingBox::ApplyTransform(const Matrix3& rotation, const Vector3& translation)
{
// 	var xa = m.Right * boundingBox.Min.X;
// 	var xb = m.Right * boundingBox.Max.X;
// 
// 	var ya = m.Up * boundingBox.Min.Y;
// 	var yb = m.Up * boundingBox.Max.Y;
// 
// 	var za = m.Backward * boundingBox.Min.Z;
// 	var zb = m.Backward * boundingBox.Max.Z;
// 
// 	return new BoundingBox(
// 		Vector3.Min(xa, xb) + Vector3.Min(ya, yb) + Vector3.Min(za, zb) + m.Translation,
// 		Vector3.Max(xa, xb) + Vector3.Max(ya, yb) + Vector3.Max(za, zb) + m.Translation
// 	);

	Matrix3 mmin(
		min.x, min.y, min.z,
		min.x, min.y, min.z,
		min.x, min.y, min.z);
	Matrix3 mmax(
		max.x, max.y, max.z,
		max.x, max.y, max.z,
		max.x, max.y, max.z);

	Matrix3 a = rotation;
	a.MulElements(mmin);
	Matrix3 b = rotation;
	b.MulElements(mmax);

	Matrix3 newMin = Matrix3::Min(a, b);
	Matrix3 newMax = Matrix3::Max(a, b);

	min = translation;
	min.Add(newMin.GetColumn(0));
	min.Add(newMin.GetColumn(1));
	min.Add(newMin.GetColumn(2));

	max = translation;
	max.Add(newMax.GetColumn(0));
	max.Add(newMax.GetColumn(1));
	max.Add(newMax.GetColumn(2));
}

void 
AxisAlignedBoundingBox::AddPoint(const Vector3& p)
{
	// TODO: use min/max functions
	// TODO: SSE
	min.x = min.x > p.x ? p.x : min.x;
	min.y = min.y > p.y ? p.y : min.y;
	min.z = min.z > p.z ? p.z : min.z;

	max.x = max.x < p.x ? p.x : max.x;
	max.y = max.y < p.y ? p.y : max.y;
	max.z = max.z < p.z ? p.z : max.z;
}

void 
AxisAlignedBoundingBox::AddAxisAlignedBoundinBox(const AxisAlignedBoundingBox& bbox)
{
	// TODO: SSE
	min.x = std::min(min.x, bbox.min.x);
	min.y = std::min(min.y, bbox.min.y);
	min.z = std::min(min.z, bbox.min.z);

	max.x = std::max(max.x, bbox.max.x);
	max.y = std::max(max.y, bbox.max.y);
	max.z = std::max(max.z, bbox.max.z);
}

Vector3 
AxisAlignedBoundingBox::ComputeExtends() const
{
	Vector3 c = max;
	c.Sub(min);
	c.Mul(0.5f);

	return c;
}

Vector3 
AxisAlignedBoundingBox::ComputeCenter() const
{
	Vector3 c = min;
	c.Add(max);
	c.Mul(0.5f);

	return c;
}

bool 
AxisAlignedBoundingBox::Contains(const Vector3& p) const
{
    if (p.x < min.x || p.x > max.x) return false;
    if (p.y < min.y || p.y > max.y) return false;
    if (p.z < min.z || p.z > max.z) return false;

    return true;
}

}