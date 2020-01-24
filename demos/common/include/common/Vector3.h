#pragma  once

// TODO: figure out a way to avoid having those two dependencies
#include <algorithm>    // for std min/max
#include <cmath>        // for sqrt, abs


#define HEPHAESTUS_VECTOR3_OPERATORS 0

namespace hephaestus
{
class Vector3
{
public:

#ifdef _WIN32
__pragma(warning(push))
__pragma(warning(disable : 4201))   // C4201: nameless union as non standard extension 
#endif
	union alignas(16) 
	{
		struct { float x, y, z; };
		float m_elems[4]; // last element is used for alignment padding
	};
#ifdef _WIN32
__pragma(warning(pop))
#endif

public:
	// constant values
	/// Vector (0,0,0)
	static const Vector3 ZERO;
	/// Vector (1,1,1)
	static const Vector3 UNARY;
	/// Vector (1,0,0)
	static const Vector3 UNIT_X;
	/// Vector (0,1,0)
	static const Vector3 UNIT_Y;
	/// Vector (0,0,1)
	static const Vector3 UNIT_Z;
	/// Vector (-1,0,0)
	static const Vector3 NEG_UNIT_X;
	/// Vector (0,-1,0)
	static const Vector3 NEG_UNIT_Y;
	/// Vector (0,0,-1)
	static const Vector3 NEG_UNIT_Z;

public:
	// construction

	/// Default constructor
	Vector3() = default;
	/// Constructor from 3 values
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	explicit Vector3(float a) : x(a), y(a), z(a) {}

public:
	// math operations
	__forceinline void Sub(const Vector3& s) { x -= s.x; y -= s.y; z -= s.z; }
	__forceinline void Add(const Vector3& s) { x += s.x; y += s.y; z += s.z; }
	__forceinline void Mul(const Vector3& s) { x *= s.x; y *= s.y; z *= s.z; }
	__forceinline void Div(const Vector3& s) { x /= s.x; y /= s.y; z /= s.z; }
	__forceinline void Sub(float a) { x -= a; y -= a; z -= a; }
	__forceinline void Add(float a) { x += a; y += a; z += a; }
	__forceinline void Mul(float a) { x *= a; y *= a; z *= a; }
	__forceinline void Div(float a) { x /= a; y /= a; z /= a; }
    __forceinline void MulAdd(float a, const Vector3 &s) { x += a * s.x; y += a * s.y; z += a * s.z; }

    // special math operations
    __forceinline void Abs() { x = std::fabsf(x); y = std::fabsf(y); z = std::fabsf(z); }
    static Vector3 Min(const Vector3& s1, const Vector3& s2) { return Vector3(std::min<float>(s1.x, s2.x), std::min<float>(s1.y, s2.y), std::min<float>(s1.z, s2.z)); }
    static Vector3 Max(const Vector3& s1, const Vector3& s2) { return Vector3(std::max<float>(s1.x, s2.x), std::max<float>(s1.y, s2.y), std::max<float>(s1.z, s2.z)); }

	/// invert components in place
	__forceinline void Invert() { x = -x; y = -y; z = -z; }
	__forceinline void InvertX() { x = -x; }
	__forceinline void InvertY() { y = -y; }
	__forceinline void InvertZ() { z = -z; }

	// Dot products
	__forceinline float Dot() const { return x * x + y * y + z * z; }
	__forceinline float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

	// Coordinates sum
	__forceinline float Sum() const { return x + y + z; }
	// Vector normalization
	__forceinline void Normalize() { Div(Norm()); }
	// Vector norm
	__forceinline float Norm() const { return std::sqrtf(Dot()); }
	// The normal in the direction of vector
	Vector3 Normal() const 
	{ 
		Vector3 temp(*this);
		temp.Normalize(); 
		return temp; 
	}
	// Distance with the other vector
	float Dist(const Vector3 &other) const
	{ 
		Vector3 temp(*this);
		temp.Sub(other);
		return temp.Norm();
	}

	// Cross product
	static Vector3 Cross(const Vector3 &s1, const Vector3 &s2)
	{
		return Vector3(s1.y*s2.z - s1.z*s2.y, s1.z*s2.x - s1.x*s2.z, s1.x*s2.y - s1.y*s2.x);
	}

public:
#if HEPHAESTUS_VECTOR3_OPERATORS
	// basic c++ operators
	/// Addition
	const Vector3 operator+(const Vector3 &s) const { return Vector3(x + s.x, y + s.y, z + s.z); }
	/// Subtraction
	const Vector3 operator-(const Vector3 &s) const { return Vector3(x - s.x, y - s.y, z - s.z); }
	/// Per coordinate multiplication
	const Vector3 operator*(const Vector3 &s) const { return Vector3(x*s.x, y*s.y, z*s.z); }
	/// Per coordinate division
	const Vector3 operator/(const Vector3 &s) const { return Vector3(x / s.x, y / s.y, z / s.z); }
	/// Division with scalar value
	const Vector3 operator/(float a) const { return Vector3(x / a, y / a, z / a); }
	/// multiplication with scalar
	const Vector3 operator*(float a) const { return Vector3(x*a, y*a, z*a); }
	/// Equality test
	bool operator==(const Vector3 &s) const { return (x == s.x && y == s.y && z == s.z); }
	/// Less than test
	bool operator<(const Vector3 &s) const { return (x < s.x && y < s.y && z < s.z); }
	/// Non equal test
	bool operator!=(const Vector3 &s) const { return !(*this == s); }
	/// In place subtraction
	void operator-=(const Vector3 &s) { x -= s.x; y -= s.y; z -= s.z; }
	/// In place addition
	void operator+=(const Vector3 &s) { x += s.x; y += s.y; z += s.z; }
	/// In place per coordinate multiplication
	void operator*=(const Vector3 &s) { x *= s.x; y *= s.y; z *= s.z; }
	/// In place per coordinate division
	void operator/=(const Vector3 &s) { x /= s.x; y /= s.y; z /= s.z; }
	/// In place subtraction with scalar
	void operator-=(float a) { x -= a; y -= a; z -= a; }
	/// In place addition with scalar
	void operator+=(float a) { x += a; y += a; z += a; }
	/// In place multiplication with scalar
	void operator*=(float a) { x *= a; y *= a; z *= a; }
	/// In place division with scalar
	void operator/=(float a) { x /= a; y /= a; z /= a; }
	/// Assignment operator
	void operator=(float a) { x = y = z = a; }
#endif

};

}
