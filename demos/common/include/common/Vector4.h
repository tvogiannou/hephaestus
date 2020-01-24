#pragma  once

#include <common/Vector3.h>

#include <cmath>    // for sqrt


namespace hephaestus
{
class Vector4
{
public:

#ifdef _WIN32
__pragma(warning(push))
__pragma(warning(disable : 4201))   // C4201: nameless union as non standard extension 
#endif
	union alignas(16)
	{
		struct { float x, y, z, w; };
		float m_elems[4];
	};
#ifdef _WIN32
__pragma(warning(pop))
#endif

public:
	// constant values
	static const Vector4 ZERO;
	static const Vector4 UNARY;
	static const Vector4 UNIT_X;
	static const Vector4 UNIT_Y;
	static const Vector4 UNIT_Z;
	static const Vector4 UNIT_W;
	static const Vector4 NEG_UNIT_X;
	static const Vector4 NEG_UNIT_Y;
	static const Vector4 NEG_UNIT_Z;
	static const Vector4 NEG_UNIT_W;

public:
	// construction
	Vector4() = default;
	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vector4(const Vector3& vec3, float _w) : x(vec3.x), y(vec3.y), z(vec3.z), w(_w) {}
	explicit Vector4(float a) : x(a), y(a), z(a), w(a) {}

public:
	// math operations
	 void Sub(const Vector4 &s) { x -= s.x; y -= s.y; z -= s.z; w -= s.w; }
	 void Add(const Vector4 &s) { x += s.x; y += s.y; z += s.z; w += s.w; }
	 void Mul(const Vector4 &s) { x *= s.x; y *= s.y; z *= s.z; w *= s.w; }
	 void Div(const Vector4 &s) { x /= s.x; y /= s.y; z /= s.z; w /= s.w; }
	 void Sub(float a) { x -= a; y -= a; z -= a; w -= a; }
	 void Add(float a) { x += a; y += a; z += a; w += a; }
	 void Mul(float a) { x *= a; y *= a; z *= a; w *= a; }
	 void Div(float a) { x /= a; y /= a; z /= a; w /= a; }

	/// invert components in place
	 void Invert() { x = -x; y = -y; z = -z; w = -w; }
	 void InvertX() { x = -x; }
	 void InvertY() { y = -y; }
	 void InvertZ() { z = -z; }
	 void InvertW() { w = -w; }

	/// Dot products
	 float Dot() const { return x * x + y * y + z * z + w * w; }
	 float Dot(const Vector4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

	/// Coordinates sum
	 float Sum() const { return x + y + z + w; }
	/// Vector normalization
	 void Normalize() { Div(Norm()); }
	/// Vector norm
	 float Norm() const { return std::sqrt(Dot()); }
	/// The normal in the direction of vector
	Vector4 Normal() const 
	{ 
		Vector4 temp(*this);
		temp.Normalize(); 
		return temp; 
	}
};

} // hephaestus
