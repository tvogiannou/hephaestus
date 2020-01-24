#include <common/Matrix3.h>

#include <algorithm>    // for std min/max
#include <cmath>        // for cos, sin
#include <cassert>


namespace hephaestus
{
const Matrix3 Matrix3::IDENTITY = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
const Matrix3 Matrix3::ZERO = Matrix3(0.0f);
const Matrix3 Matrix3::UNARY = Matrix3(1.0f);

// static
const Matrix3
Matrix3::OutProduct(const Vector3 &v1, const Vector3 &v2)
{
	Matrix3 m;
	m.m00 = v1.x * v2.x;	m.m01 = v1.x * v2.y;	m.m02 = v1.x * v2.z;
	m.m10 = v1.y * v2.x;	m.m11 = v1.y * v2.y;	m.m12 = v1.y * v2.z;
	m.m20 = v1.z * v2.x;	m.m21 = v1.z * v2.y;	m.m22 = v1.z * v2.z;
	return m;
}

Matrix3 
Matrix3::Min(const Matrix3& a, const Matrix3& b)
{
	return Matrix3(
		std::min(a.m00, b.m00), std::min(a.m01, b.m01), std::min(a.m02, b.m02),
		std::min(a.m10, b.m10), std::min(a.m11, b.m11), std::min(a.m12, b.m12),
		std::min(a.m20, b.m20), std::min(a.m21, b.m21), std::min(a.m22, b.m22));
}

Matrix3 
Matrix3::Max(const Matrix3& a, const Matrix3& b)
{
	return Matrix3(
		std::max(a.m00, b.m00), std::max(a.m01, b.m01), std::max(a.m02, b.m02),
		std::max(a.m10, b.m10), std::max(a.m11, b.m11), std::max(a.m12, b.m12),
		std::max(a.m20, b.m20), std::max(a.m21, b.m21), std::max(a.m22, b.m22));
}

void 
Matrix3::Set(uint32_t i, uint32_t j, float v)
{ 
	assert(i < 3 && j < 3); 
	m_elems[i][j] = v; 
}

float 
Matrix3::Get(uint32_t i, uint32_t j) const 
{ 
	assert(i < 3 && j < 3); 
	return m_elems[i][j]; 
}

Vector3 
Matrix3::GetColumn(uint32_t col) const
{
	assert(col < 3);

	return Vector3(
		m_elems[0][col], 
		m_elems[1][col], 
		m_elems[2][col]);
}

Vector3 
Matrix3::GetRow(uint32_t row) const
{
	assert(row < 3);

	return Vector3(m_elems[row][0], m_elems[row][1], m_elems[row][2]);
}

void 
Matrix3::Set(float v)
{
	m00 = m01 = m02 = m10 =
		m11 = m12 = m20 = m21 = m22 = v;
}

void 
Matrix3::SetDiagonal(float v)
{
	m00 = m11 = m22 = v;
}

void 
Matrix3::SetDiagonal(const Vector3& v)
{
	m00 = v.x; 
	m11 = v.y;
	m22 = v.z;
}

void 
Matrix3::SetIdentity()
{
	m00 = m11 = m22 = 1.0f;
	m01 = m02 = m10 = m12 = m20 = m21 = 0.0f;
}

float
Matrix3::Determinant() const
{
	return
		m00 * (m22 * m11 - m21 * m12) -
		m10 * (m22 * m01 - m21 * m02) +
		m20 * (m12 * m01 - m11 * m02);
}

void 
Matrix3::Add(const Matrix3 &s)
{
	m00 += s.m00;	m01 += s.m01;	m02 += s.m02;
	m10 += s.m10;	m11 += s.m11;	m12 += s.m12;
	m20 += s.m20;	m21 += s.m21;	m22 += s.m22;
}

void 
Matrix3::Sub(const Matrix3 &s)
{
	m00 -= s.m00;	m01 -= s.m01;	m02 -= s.m02;
	m10 -= s.m10;	m11 -= s.m11;	m12 -= s.m12;
	m20 -= s.m20;	m21 -= s.m21;	m22 -= s.m22;
}

void 
Matrix3::MulElements(const Matrix3 &s)
{
	m00 *= s.m00;	m01 *= s.m01;	m02 *= s.m02;
	m10 *= s.m10;	m11 *= s.m11;	m12 *= s.m12;
	m20 *= s.m20;	m21 *= s.m21;	m22 *= s.m22;
}

void 
Matrix3::DivElements(const Matrix3 &s)
{
	m00 /= s.m00;	m01 /= s.m01;	m02 /= s.m02;
	m10 /= s.m10;	m11 /= s.m11;	m12 /= s.m12;
	m20 /= s.m20;	m21 /= s.m21;	m22 /= s.m22;
}

void 
Matrix3::Mul(float a)
{
	m00 *= a;	m01 *= a;	m02 *= a;
	m10 *= a;	m11 *= a;	m12 *= a;
	m20 *= a;	m21 *= a;	m22 *= a;
}

void 
Matrix3::Div(float a)
{
	m00 /= a;	m01 /= a;	m02 /= a;
	m10 /= a;	m11 /= a;	m12 /= a;
	m20 /= a;	m21 /= a;	m22 /= a;
}

const Matrix3
Matrix3::GetMul(const Matrix3 &s) const
{
	return Matrix3(
		m00 * s.m00 + m01 * s.m10 + m02 * s.m20,
		m00 * s.m01 + m01 * s.m11 + m02 * s.m21,
		m00 * s.m02 + m01 * s.m12 + m02 * s.m22,

		m10 * s.m00 + m11 * s.m10 + m12 * s.m20,
		m10 * s.m01 + m11 * s.m11 + m12 * s.m21,
		m10 * s.m02 + m11 * s.m12 + m12 * s.m22,

		m20 * s.m00 + m21 * s.m10 + m22 * s.m20,
		m20 * s.m01 + m21 * s.m11 + m22 * s.m21,
		m20 * s.m02 + m21 * s.m12 + m22 * s.m22);
}

void
Matrix3::Mul(const Matrix3 &s)
{
	*this = GetMul(s);
}

Vector3 
Matrix3::MulVec3Right(const Vector3 &v) const
{
	return Vector3(
		m00 * v.x + m01 * v.y + m02 * v.z,
		m10 * v.x + m11 * v.y + m12 * v.z,
		m20 * v.x + m21 * v.y + m22 * v.z);
}

Vector3 
Matrix3::MulVec3Left(const Vector3 &v) const
{
	return Vector3(
		m00 * v.x + m10 * v.y + m20 * v.z,
		m01 * v.x + m11 * v.y + m21 * v.z,
		m02 * v.x + m12 * v.y + m22 * v.z);
}

void 
Matrix3::Transpose()
{
	std::swap(m01, m10);
	std::swap(m02, m20);
	std::swap(m21, m12);
}

/// Get the transpose matrix
const Matrix3 
Matrix3::GetTranspose() const
{
	Matrix3 r(*this);
	r.Transpose();
	return r;
}

/// Get the inverse matrix
const Matrix3 
Matrix3::GetInverse() const
{
	Matrix3 r(0.0f);
	// DET = a00(a22a11-a21a12)-a10(a22a01-a21a02)+a20(a12a01-a11a02)
	const float det = Determinant();
	// 		const float det = 
	// 			data[0][0] * (data[2][2] * data[1][1] - data[2][1] * data[1][2]) -
	// 			data[1][0] * (data[2][2] * data[0][1] - data[2][1] * data[0][2]) +
	// 			data[2][0] * (data[1][2] * data[0][1] - data[1][1] * data[0][2]);

			// 		 |   a22a11-a21a12  -(a22a01-a21a02)   a12a01-a11a02  |
			// 		 | -(a22a10-a20a12)   a22a00-a20a02  -(a12a00-a10a02) |
			// 		 |   a21a10-a20a11  -(a21a00-a20a01)   a11a00-a10a01  |

	r.m00 = m22 * m11 - m21 * m12;
	r.m01 = -(m22 * m01 - m21 * m02);
	r.m02 = m12 * m01 - m11 * m02;

	r.m10 = -(m22 * m10 - m20 * m12);
	r.m11 = m22 * m00 - m20 * m02;
	r.m12 = -(m12 * m00 - m10 * m02);

	r.m20 = m21 * m10 - m20 * m11;
	r.m21 = -(m21 * m00 - m20 * m01);
	r.m22 = m11 * m00 - m10 * m01;

	r.Mul(1.0f / det);

	return r;
}

// creation from rotation axis

/// Create rotation matrix for angle rotation along X axis
void 
Matrix3::SetFromRotationAxisX(float rads)
{
	const float cos_a = std::cos(rads);
	const float sin_a = std::sin(rads);
	m00 = 1.0f;	m01 = 0.0f;	m02 = 0.0f;
	m10 = 0.0f;	m11 = cos_a;	m12 = -sin_a;
	m20 = 0.0f;	m21 = sin_a;	m22 = cos_a;
}

/// Create rotation matrix for angle rotation along Y axis
void 
Matrix3::SetFromRotationAxisY(float rads)
{
	const float cos_a = std::cos(rads);
	const float sin_a = std::sin(rads);
	m00 = cos_a;	 m01 = 0.0f;	m02 = sin_a;
	m10 = 0.0f;	 m11 = 1.0f;	m12 = 0.0f;
	m20 = -sin_a; m21 = 0.0f;	m22 = cos_a;
}

/// Create rotation matrix for angle rotation along X axis
void 
Matrix3::SetFromRotationAxisZ(float rads)
{
	const float cos_a = std::cos(rads);
	const float sin_a = std::sin(rads);
	m00 = cos_a;	m01 = -sin_a; m02 = 0.0f;
	m10 = sin_a;	m11 = cos_a;	 m12 = 0.0f;
	m20 = 0.0f;	m21 = 0.0f;	 m22 = 1.0f;
}

void Matrix3::SetFromRotationAxis(const Vector3& axis, float rads)
{
	Vector3 u = axis.Normal();

    float sinAngle = std::sin(rads);
    float cosAngle = std::cos(rads);
	float oneMinusCosAngle = 1.0f - cosAngle;

	m00 = (u.x)*(u.x) + cosAngle * (1 - (u.x)*(u.x));
	m01 = (u.x)*(u.y)*(oneMinusCosAngle)-sinAngle * u.z;
	m02 = (u.x)*(u.z)*(oneMinusCosAngle)+sinAngle * u.y;

	m10 = (u.x)*(u.y)*(oneMinusCosAngle)+sinAngle * u.z;
	m11 = (u.y)*(u.y) + cosAngle * (1 - (u.y)*(u.y));
	m12 = (u.y)*(u.z)*(oneMinusCosAngle)-sinAngle * u.x;

	m20 = (u.x)*(u.z)*(oneMinusCosAngle)-sinAngle * u.y;
	m21 = (u.y)*(u.z)*(oneMinusCosAngle)+sinAngle * u.x;
	m22 = (u.z)*(u.z) + cosAngle * (1 - (u.z)*(u.z));
}

} // hephaestus

