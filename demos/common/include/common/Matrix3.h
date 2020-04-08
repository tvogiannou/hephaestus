#pragma once

#include <hephaestus/Compiler.h>
#include <common/Vector3.h>


namespace hephaestus
{
class Matrix3
{
private:

#ifdef _WIN32
__pragma(warning(push))
__pragma(warning(disable : 4201))   // C4201: nameless union as non standard extension 
#endif
    union alignas(16)
    {
        struct { float m00, m01, m02, m10, m11, m12, m20, m21, m22; };
        float  m_elems[4][3];	// last row is used for alignment padding
    };
#ifdef _WIN32
__pragma(warning(pop))
#endif

public:
    // constant matrices

    /// Identity matrix
    static const Matrix3 IDENTITY;
    /// Zero matrix
    static const Matrix3 ZERO;
    /// Matrix with every element equal to 1
    static const Matrix3 UNARY;

public:

    Matrix3() = default;

    /// Create matrix with all values equal to v
    explicit Matrix3(float v) { Set(v); }

    /// Create matrix from 9 values
    Matrix3(
        float _m00, float _m01, float _m02,
        float _m10, float _m11, float _m12,
        float _m20, float _m21, float _m22)
    {
        m00 = _m00;	m01 = _m01;	m02 = _m02;
        m10 = _m10;	m11 = _m11;	m12 = _m12;
        m20 = _m20;	m21 = _m21;	m22 = _m22;
    }

    /// Copy data from C 3x3 array
    void CopyFromArray(const float v[3][3])
    {
        m00 = v[0][0];	m01 = v[0][1];	m02 = v[0][2];
        m10 = v[1][0];	m11 = v[1][1];	m12 = v[1][2];
        m20 = v[2][0];	m21 = v[2][1];	m22 = v[2][2];
    }

public:

    // Setters/Getters
    float Get(uint32_t i, uint32_t j) const;		// Get the value at the (i,j) position
    Vector3 GetColumn(uint32_t col) const;			// Get column as a Vector3
    Vector3 GetRow(uint32_t row) const;				// Get row as a Vector3
    void Set(uint32_t i, uint32_t j, float v);		// Set matrix value at the (i,j) position
    void Set(float v);								// Set all the matrix elements equal to v
    void SetDiagonal(float v);						// Set the diagonal of the matrix equal to v
    void SetDiagonal(const Vector3& v);				// Set the diagonal of the matrix equal to v
    void SetIdentity();								// Set matrix to identity matrix

    // element wise, in-place math operations
    void Add(const Matrix3 &s);						// In place per element matrix addition
    void Sub(const Matrix3 &s);						// In place per element matrix subtraction
    void MulElements(const Matrix3& s);				// In place per element matrix multiplication
    void DivElements(const Matrix3& s);				// In place per element matrix subtraction
    void Div(float a);								// Right in place division with scalar
    void Mul(float a);								// Right in place multiplication with scalar

    // matrix products
    const Matrix3 GetMul(const Matrix3 &s) const;	// Matrix multiplication, returns the resulting matrix
    void Mul(const Matrix3 &s);						// In place matrix multiplication
    Vector3 MulVec3Right(const Vector3 &v) const;	// Right multiplication with vector (treat as column vector)
    Vector3 MulVec3Left(const Vector3 &v) const;	// Left multiplication with vector (treat as row vector)

        /// Create a matrix as the outer product between two vectors
    static const Matrix3 OutProduct(const Vector3 &v1, const Vector3 &v2);

    static Matrix3 Min(const Matrix3& a, const Matrix3& b);
    static Matrix3 Max(const Matrix3& a, const Matrix3& b);

public:
    // matrix manipulation

    /// Set the matrix to its transpose
    void Transpose();

    /// Get the transpose matrix
    const Matrix3 GetTranspose() const;

    /// Get the determinant of the matrix
    float Determinant() const;

    /// Get the inverse matrix
    const Matrix3 GetInverse() const;

    /// Create rotation matrix for angle rotation along an axis
    void SetFromRotationAxisX(float rads);
    void SetFromRotationAxisY(float rads);
    void SetFromRotationAxisZ(float rads);
    void SetFromRotationAxis(const Vector3& axis, float rads);

};

}

