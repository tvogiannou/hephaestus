#pragma once

#include <common/Vector3.h>

#include <array>


namespace hephaestus
{
class Matrix3;
class Vector4;

class Matrix4
{
    // column major, indexed as
    // [ 0	4   8  12 ]
    // [ 1	5   9  13 ]
    // [ 2	6  10  14 ]
    // [ 3	7  11  15 ]
    alignas(16) std::array<float, 16> m_elems;

public:
    Matrix4() = default;
    Matrix4(
        float e0, float e1, float e2, float e3,
        float e4, float e5, float e6, float e7,
        float e8, float e9, float e10, float e11,
        float e12, float e13, float e14, float e15);
    explicit Matrix4(const std::array<float, 16>& elements);

    void SetElement(uint32_t index, float value);
    float GetElement(uint32_t index) const;
    float* GetRaw() { return m_elems.data(); }
    const float* GetRaw() const { return const_cast<Matrix4*>(this)->GetRaw(); }
    void GetRaw(std::array<float, 16>& raw) const { raw = m_elems; }

    void GetRow(uint32_t index, std::array<float, 4>& output) const;
    void GetRow(uint32_t index, Vector4& output) const;
    void GetColumn(uint32_t index, std::array<float, 4>& output) const;
    void GetColumn(uint32_t index, Vector4& output) const;

    void SetIdentity();
    void SetZero();
    void SetDiagonal(float v);
    void SetDiagonal(const Vector4& v);

public:
    // in-place, element wise operations
    void Add(const Matrix4& other);
    void Sub(const Matrix4& other);
    void MulElements(const Matrix4& other);
    void DivElements(const Matrix4& other);
    void Mul(float value);
    void Div(float value);

    // matrix multiplication
    Matrix4 GetMul(const Matrix4& other) const;
    Vector4 MulVec4Right(const Vector4& other) const; // TEST

    // invert and transpose
    void Inverse();
    void Transpose();
    void InvertTranspose();
    Matrix4 GetInverse() const;
    Matrix4 GetTranspose() const;
    Matrix4 GetInverseTranspose() const;

public:
    // API for manipulating the 4x4 matrix as if it represents an Affine transform
    // [0:3] [4:7] [8:11] [12:15]
    //  Rx    Ry    Rz     T
    //  0     0     0      1
    //
    // where Rx, Ry, Rz are the unit (column) vectors defined by the rotation frame
    // and T is the translation (column) vector

    Vector3 AffineMulVec3Right(const Vector3& other) const;
    Matrix4 AffineGetMul(const Matrix4& affineOther) const;
    void AffineInverse();
    //Matrix4 AffineGetInverse() const;

    void AffineGetTranslation(Vector3& translation) const ;
    void AffineGetRotation(Matrix3& rotation) const;

    void AffineSet();
    void AffineSetTranslation(const Vector3& translation);
    void AffineSetScale(const Vector3& scaleFactor);
    void AffineSetUniformScale(float scaleFactor);
    void AffineSetRotation(const Matrix3& rotation);
};

} // hephaestus

