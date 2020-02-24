
#include <common/Matrix4.h>

#include <common/Matrix3.h>
#include <common/Vector4.h>

#include <utility> // for std swap
#include <cassert>

namespace hephaestus
{

Matrix4::Matrix4(float e0, float e1, float e2, float e3,
                     float e4, float e5, float e6, float e7,
                     float e8, float e9, float e10, float e11,
                     float e12, float e13, float e14, float e15)
{
    m_elems[0] = e0;
    m_elems[1] = e1;
    m_elems[2] = e2;
    m_elems[3] = e3;
    m_elems[4] = e4;
    m_elems[5] = e5;
    m_elems[6] = e6;
    m_elems[7] = e7;
    m_elems[8] = e8;
    m_elems[9] = e9;
    m_elems[10] = e10;
    m_elems[11] = e11;
    m_elems[12] = e12;
    m_elems[13] = e13;
    m_elems[14] = e14;
    m_elems[15] = e15;
}

Matrix4::Matrix4(const std::array<float, 16>& elements)
{
    m_elems = elements;
}

void
Matrix4::SetElement(uint32_t index, float value)
{
    m_elems.at(index) = value;
}

float 
Matrix4::GetElement(uint32_t index) const
{
    return m_elems.at(index);
}

void 
Matrix4::GetRow(uint32_t index, std::array<float, 4>& output) const
{
    assert(index < 4);

    output[0] = m_elems[index];
    output[1] = m_elems[4 + index];
    output[2] = m_elems[8 + index];
    output[3] = m_elems[12 + index];
}

void 
Matrix4::GetRow(uint32_t index, Vector4& output) const
{
    assert(index < 4);

    output.x = m_elems[index];
    output.y = m_elems[4 + index];
    output.z = m_elems[8 + index];
    output.w = m_elems[12 + index];
}

void 
Matrix4::GetColumn(uint32_t index, std::array<float, 4>& output) const
{
    assert(index < 4);

    const int offset = 4*index;
    output[0] = m_elems[offset];
    output[1] = m_elems[offset + 1];
    output[2] = m_elems[offset + 2];
    output[3] = m_elems[offset + 3];
}

void 
Matrix4::GetColumn(uint32_t index, Vector4& output) const
{
    assert(index < 4);

    const int offset = 4 * index;
    output.x = m_elems[offset];
    output.y = m_elems[offset + 1];
    output.z = m_elems[offset + 2];
    output.w = m_elems[offset + 3];
}

void 
Matrix4::SetIdentity(void)
{
    SetZero();

    m_elems[0] = 1.0f;
    m_elems[5] = 1.0f;
    m_elems[10] = 1.0f;
    m_elems[15] = 1.0f;
}

void 
Matrix4::SetZero(void)
{
    m_elems.fill(0.0f);
}

void 
Matrix4::SetDiagonal(float v)
{
    m_elems[0] = v;
    m_elems[5] = v;
    m_elems[10] = v;
    m_elems[15] = v;
}

void 
Matrix4::SetDiagonal(const Vector4& v)
{
    m_elems[0] = v.x;
    m_elems[5] = v.y;
    m_elems[10] = v.z;
    m_elems[15] = v.w;
}

void 
Matrix4::Add(const Matrix4 & other)
{
    m_elems[0]	+= other.m_elems[0];
    m_elems[1]	+= other.m_elems[1];
    m_elems[2]	+= other.m_elems[2];
    m_elems[3]	+= other.m_elems[3];
    m_elems[4]	+= other.m_elems[4];
    m_elems[5]	+= other.m_elems[5];
    m_elems[6]	+= other.m_elems[6];
    m_elems[7]	+= other.m_elems[7];
    m_elems[8]	+= other.m_elems[8];
    m_elems[9]	+= other.m_elems[9];
    m_elems[10] += other.m_elems[10];
    m_elems[11] += other.m_elems[11];
    m_elems[12] += other.m_elems[12];
    m_elems[13] += other.m_elems[13];
    m_elems[14] += other.m_elems[14];
    m_elems[15] += other.m_elems[15];
}

void 
Matrix4::Sub(const Matrix4 & other)
{
    m_elems[0] -= other.m_elems[0];
    m_elems[1] -= other.m_elems[1];
    m_elems[2] -= other.m_elems[2];
    m_elems[3] -= other.m_elems[3];
    m_elems[4] -= other.m_elems[4];
    m_elems[5] -= other.m_elems[5];
    m_elems[6] -= other.m_elems[6];
    m_elems[7] -= other.m_elems[7];
    m_elems[8] -= other.m_elems[8];
    m_elems[9] -= other.m_elems[9];
    m_elems[10] -= other.m_elems[10];
    m_elems[11] -= other.m_elems[11];
    m_elems[12] -= other.m_elems[12];
    m_elems[13] -= other.m_elems[13];
    m_elems[14] -= other.m_elems[14];
    m_elems[15] -= other.m_elems[15];
}

void 
Matrix4::MulElements(const Matrix4 & other)
{
    m_elems[0] *= other.m_elems[0];
    m_elems[1] *= other.m_elems[1];
    m_elems[2] *= other.m_elems[2];
    m_elems[3] *= other.m_elems[3];
    m_elems[4] *= other.m_elems[4];
    m_elems[5] *= other.m_elems[5];
    m_elems[6] *= other.m_elems[6];
    m_elems[7] *= other.m_elems[7];
    m_elems[8] *= other.m_elems[8];
    m_elems[9] *= other.m_elems[9];
    m_elems[10] *= other.m_elems[10];
    m_elems[11] *= other.m_elems[11];
    m_elems[12] *= other.m_elems[12];
    m_elems[13] *= other.m_elems[13];
    m_elems[14] *= other.m_elems[14];
    m_elems[15] *= other.m_elems[15];
}

void 
Matrix4::DivElements(const Matrix4& other)
{
    m_elems[0] /= other.m_elems[0];
    m_elems[1] /= other.m_elems[1];
    m_elems[2] /= other.m_elems[2];
    m_elems[3] /= other.m_elems[3];
    m_elems[4] /= other.m_elems[4];
    m_elems[5] /= other.m_elems[5];
    m_elems[6] /= other.m_elems[6];
    m_elems[7] /= other.m_elems[7];
    m_elems[8] /= other.m_elems[8];
    m_elems[9] /= other.m_elems[9];
    m_elems[10] /= other.m_elems[10];
    m_elems[11] /= other.m_elems[11];
    m_elems[12] /= other.m_elems[12];
    m_elems[13] /= other.m_elems[13];
    m_elems[14] /= other.m_elems[14];
    m_elems[15] /= other.m_elems[15];
}

void 
Matrix4::Mul(float value)
{
    m_elems[0] *= value;
    m_elems[1] *= value;
    m_elems[2] *= value;
    m_elems[3] *= value;
    m_elems[4] *= value;
    m_elems[5] *= value;
    m_elems[6] *= value;
    m_elems[7] *= value;
    m_elems[8] *= value;
    m_elems[9] *= value;
    m_elems[10] *= value;
    m_elems[11] *= value;
    m_elems[12] *= value;
    m_elems[13] *= value;
    m_elems[14] *= value;
    m_elems[15] *= value;
}

Matrix4
Matrix4::GetMul(const Matrix4& other) const
{
    return Matrix4(
        m_elems[0] * other.m_elems[0] + m_elems[4] * other.m_elems[1] + m_elems[8] * other.m_elems[2] + m_elems[12] * other.m_elems[3],
        m_elems[1] * other.m_elems[0] + m_elems[5] * other.m_elems[1] + m_elems[9] * other.m_elems[2] + m_elems[13] * other.m_elems[3],
        m_elems[2] * other.m_elems[0] + m_elems[6] * other.m_elems[1] + m_elems[10] * other.m_elems[2] + m_elems[14] * other.m_elems[3],
        m_elems[3] * other.m_elems[0] + m_elems[7] * other.m_elems[1] + m_elems[11] * other.m_elems[2] + m_elems[15] * other.m_elems[3],
        m_elems[0] * other.m_elems[4] + m_elems[4] * other.m_elems[5] + m_elems[8] * other.m_elems[6] + m_elems[12] * other.m_elems[7],
        m_elems[1] * other.m_elems[4] + m_elems[5] * other.m_elems[5] + m_elems[9] * other.m_elems[6] + m_elems[13] * other.m_elems[7],
        m_elems[2] * other.m_elems[4] + m_elems[6] * other.m_elems[5] + m_elems[10] * other.m_elems[6] + m_elems[14] * other.m_elems[7],
        m_elems[3] * other.m_elems[4] + m_elems[7] * other.m_elems[5] + m_elems[11] * other.m_elems[6] + m_elems[15] * other.m_elems[7],
        m_elems[0] * other.m_elems[8] + m_elems[4] * other.m_elems[9] + m_elems[8] * other.m_elems[10] + m_elems[12] * other.m_elems[11],
        m_elems[1] * other.m_elems[8] + m_elems[5] * other.m_elems[9] + m_elems[9] * other.m_elems[10] + m_elems[13] * other.m_elems[11],
        m_elems[2] * other.m_elems[8] + m_elems[6] * other.m_elems[9] + m_elems[10] * other.m_elems[10] + m_elems[14] * other.m_elems[11],
        m_elems[3] * other.m_elems[8] + m_elems[7] * other.m_elems[9] + m_elems[11] * other.m_elems[10] + m_elems[15] * other.m_elems[11],
        m_elems[0] * other.m_elems[12] + m_elems[4] * other.m_elems[13] + m_elems[8] * other.m_elems[14] + m_elems[12] * other.m_elems[15],
        m_elems[1] * other.m_elems[12] + m_elems[5] * other.m_elems[13] + m_elems[9] * other.m_elems[14] + m_elems[13] * other.m_elems[15],
        m_elems[2] * other.m_elems[12] + m_elems[6] * other.m_elems[13] + m_elems[10] * other.m_elems[14] + m_elems[14] * other.m_elems[15],
        m_elems[3] * other.m_elems[12] + m_elems[7] * other.m_elems[13] + m_elems[11] * other.m_elems[14] + m_elems[15] * other.m_elems[15]);
}

Vector4 
Matrix4::MulVec4Right(const Vector4& other) const
{
    return Vector4(
        m_elems[0] * other.x + m_elems[4] * other.y + m_elems[8] * other.z + m_elems[12] * other.w,
        m_elems[1] * other.x + m_elems[5] * other.y + m_elems[9] * other.z + m_elems[13] * other.w,
        m_elems[2] * other.x + m_elems[6] * other.y + m_elems[10] * other.z + m_elems[14] * other.w,
        m_elems[3] * other.x + m_elems[7] * other.y + m_elems[11] * other.z + m_elems[15] * other.w
    );
}

void 
Matrix4::Div(float value)
{
    m_elems[0] /= value;
    m_elems[1] /= value;
    m_elems[2] /= value;
    m_elems[3] /= value;
    m_elems[4] /= value;
    m_elems[5] /= value;
    m_elems[6] /= value;
    m_elems[7] /= value;
    m_elems[8] /= value;
    m_elems[9] /= value;
    m_elems[10] /= value;
    m_elems[11] /= value;
    m_elems[12] /= value;
    m_elems[13] /= value;
    m_elems[14] /= value;
    m_elems[15] /= value;
}

void 
Matrix4::Inverse(void)
{
    *this = GetInverse();
}

Matrix4 
Matrix4::GetInverse(void) const
{
    Matrix4 result = GetInverseTranspose();
    result.Transpose();

    return result;
}

void 
Matrix4::Transpose(void)
{
    std::swap(m_elems[1], m_elems[4]);
    std::swap(m_elems[2], m_elems[8]);
    std::swap(m_elems[6], m_elems[9]);
    std::swap(m_elems[3], m_elems[12]);
    std::swap(m_elems[7], m_elems[13]);
    std::swap(m_elems[11], m_elems[14]);
}

Matrix4 
Matrix4::GetTranspose(void) const
{
    return Matrix4(	
        m_elems[ 0], m_elems[ 4], m_elems[ 8], m_elems[12],
        m_elems[ 1], m_elems[ 5], m_elems[ 9], m_elems[13],
        m_elems[ 2], m_elems[ 6], m_elems[10], m_elems[14],
        m_elems[ 3], m_elems[ 7], m_elems[11], m_elems[15]);
}

void 
Matrix4::InvertTranspose(void)
{
    *this = GetInverseTranspose();
}

Matrix4 
Matrix4::GetInverseTranspose(void) const
{
    Matrix4 result;

    float tmp[12];												//temporary pair storage
    float det;													//determinant

    //calculate pairs for first 8 elements (cofactors)
    tmp[0] = m_elems[10] * m_elems[15];
    tmp[1] = m_elems[11] * m_elems[14];
    tmp[2] = m_elems[9] * m_elems[15];
    tmp[3] = m_elems[11] * m_elems[13];
    tmp[4] = m_elems[9] * m_elems[14];
    tmp[5] = m_elems[10] * m_elems[13];
    tmp[6] = m_elems[8] * m_elems[15];
    tmp[7] = m_elems[11] * m_elems[12];
    tmp[8] = m_elems[8] * m_elems[14];
    tmp[9] = m_elems[10] * m_elems[12];
    tmp[10] = m_elems[8] * m_elems[13];
    tmp[11] = m_elems[9] * m_elems[12];

    //calculate first 8 elements (cofactors)
    result.SetElement(0,		tmp[0]*m_elems[5] + tmp[3]*m_elems[6] + tmp[4]*m_elems[7]
    -	tmp[1]*m_elems[5] - tmp[2]*m_elems[6] - tmp[5]*m_elems[7]);

    result.SetElement(1,		tmp[1]*m_elems[4] + tmp[6]*m_elems[6] + tmp[9]*m_elems[7]
    -	tmp[0]*m_elems[4] - tmp[7]*m_elems[6] - tmp[8]*m_elems[7]);

    result.SetElement(2,		tmp[2]*m_elems[4] + tmp[7]*m_elems[5] + tmp[10]*m_elems[7]
    -	tmp[3]*m_elems[4] - tmp[6]*m_elems[5] - tmp[11]*m_elems[7]);

    result.SetElement(3,		tmp[5]*m_elems[4] + tmp[8]*m_elems[5] + tmp[11]*m_elems[6]
    -	tmp[4]*m_elems[4] - tmp[9]*m_elems[5] - tmp[10]*m_elems[6]);

    result.SetElement(4,		tmp[1]*m_elems[1] + tmp[2]*m_elems[2] + tmp[5]*m_elems[3]
    -	tmp[0]*m_elems[1] - tmp[3]*m_elems[2] - tmp[4]*m_elems[3]);

    result.SetElement(5,		tmp[0]*m_elems[0] + tmp[7]*m_elems[2] + tmp[8]*m_elems[3]
    -	tmp[1]*m_elems[0] - tmp[6]*m_elems[2] - tmp[9]*m_elems[3]);

    result.SetElement(6,		tmp[3]*m_elems[0] + tmp[6]*m_elems[1] + tmp[11]*m_elems[3]
    -	tmp[2]*m_elems[0] - tmp[7]*m_elems[1] - tmp[10]*m_elems[3]);

    result.SetElement(7,		tmp[4]*m_elems[0] + tmp[9]*m_elems[1] + tmp[10]*m_elems[2]
    -	tmp[5]*m_elems[0] - tmp[8]*m_elems[1] - tmp[11]*m_elems[2]);

    //calculate pairs for second 8 elements (cofactors)
    tmp[0] = m_elems[2]*m_elems[7];
    tmp[1] = m_elems[3]*m_elems[6];
    tmp[2] = m_elems[1]*m_elems[7];
    tmp[3] = m_elems[3]*m_elems[5];
    tmp[4] = m_elems[1]*m_elems[6];
    tmp[5] = m_elems[2]*m_elems[5];
    tmp[6] = m_elems[0]*m_elems[7];
    tmp[7] = m_elems[3]*m_elems[4];
    tmp[8] = m_elems[0]*m_elems[6];
    tmp[9] = m_elems[2]*m_elems[4];
    tmp[10] = m_elems[0]*m_elems[5];
    tmp[11] = m_elems[1]*m_elems[4];

    //calculate second 8 elements (cofactors)
    result.SetElement(8,		tmp[0]*m_elems[13] + tmp[3]*m_elems[14] + tmp[4]*m_elems[15]
    -	tmp[1]*m_elems[13] - tmp[2]*m_elems[14] - tmp[5]*m_elems[15]);

    result.SetElement(9,		tmp[1]*m_elems[12] + tmp[6]*m_elems[14] + tmp[9]*m_elems[15]
    -	tmp[0]*m_elems[12] - tmp[7]*m_elems[14] - tmp[8]*m_elems[15]);

    result.SetElement(10,		tmp[2]*m_elems[12] + tmp[7]*m_elems[13] + tmp[10]*m_elems[15]
    -	tmp[3]*m_elems[12] - tmp[6]*m_elems[13] - tmp[11]*m_elems[15]);

    result.SetElement(11,		tmp[5]*m_elems[12] + tmp[8]*m_elems[13] + tmp[11]*m_elems[14]
    -	tmp[4]*m_elems[12] - tmp[9]*m_elems[13] - tmp[10]*m_elems[14]);

    result.SetElement(12,		tmp[2]*m_elems[10] + tmp[5]*m_elems[11] + tmp[1]*m_elems[9]
    -	tmp[4]*m_elems[11] - tmp[0]*m_elems[9] - tmp[3]*m_elems[10]);

    result.SetElement(13,		tmp[8]*m_elems[11] + tmp[0]*m_elems[8] + tmp[7]*m_elems[10]
    -	tmp[6]*m_elems[10] - tmp[9]*m_elems[11] - tmp[1]*m_elems[8]);

    result.SetElement(14,		tmp[6]*m_elems[9] + tmp[11]*m_elems[11] + tmp[3]*m_elems[8]
    -	tmp[10]*m_elems[11] - tmp[2]*m_elems[8] - tmp[7]*m_elems[9]);

    result.SetElement(15,		tmp[10]*m_elems[10] + tmp[4]*m_elems[8] + tmp[9]*m_elems[9]
    -	tmp[8]*m_elems[9] - tmp[11]*m_elems[10] - tmp[5]*m_elems[8]);

    // calculate determinant
    det	= m_elems[0]*result.GetElement(0)
        + m_elems[1]*result.GetElement(1)
        + m_elems[2]*result.GetElement(2)
        + m_elems[3]*result.GetElement(3);

    if(det==0.0f)
    {
        Matrix4 id;
        id.SetZero();
        return id;
    }

    result.Div(det);

    return result;
}

Vector3
Matrix4::AffineMulVec3Right(const Vector3& other) const
{
    // assume other.w = 1
    return Vector3(
        m_elems[0] * other.x + m_elems[4] * other.y + m_elems[8] * other.z + m_elems[12],
        m_elems[1] * other.x + m_elems[5] * other.y + m_elems[9] * other.z + m_elems[13],
        m_elems[2] * other.x + m_elems[6] * other.y + m_elems[10] * other.z + m_elems[14]
    );
}

Matrix4 
Matrix4::AffineGetMul(const Matrix4& affineOther) const
{
    // assume that both matrices are affine transforms therefore the last row is [0 0 0 1]
    return Matrix4(
        m_elems[0] * affineOther.m_elems[0] + m_elems[4] * affineOther.m_elems[1] + m_elems[8] * affineOther.m_elems[2] /*+ m_elems[12] * affineOther.m_elems[3]*/,
        m_elems[1] * affineOther.m_elems[0] + m_elems[5] * affineOther.m_elems[1] + m_elems[9] * affineOther.m_elems[2] /*+ m_elems[13] * affineOther.m_elems[3]*/,
        m_elems[2] * affineOther.m_elems[0] + m_elems[6] * affineOther.m_elems[1] + m_elems[10] * affineOther.m_elems[2] /*+ m_elems[14] * affineOther.m_elems[3]*/,
        0.f,///*m_elems[3] * affineOther.m_elems[0]*/ + m_elems[7] * affineOther.m_elems[1] + m_elems[11] * affineOther.m_elems[2] /*+ m_elems[15] * affineOther.m_elems[3]*/,
        m_elems[0] * affineOther.m_elems[4] + m_elems[4] * affineOther.m_elems[5] + m_elems[8] * affineOther.m_elems[6] /*+ m_elems[12] * affineOther.m_elems[7]*/,
        m_elems[1] * affineOther.m_elems[4] + m_elems[5] * affineOther.m_elems[5] + m_elems[9] * affineOther.m_elems[6] /*+ m_elems[13] * affineOther.m_elems[7]*/,
        m_elems[2] * affineOther.m_elems[4] + m_elems[6] * affineOther.m_elems[5] + m_elems[10] * affineOther.m_elems[6] /*+ m_elems[14] * affineOther.m_elems[7]*/,
        0.f,///*m_elems[3] * affineOther.m_elems[4]*/ + m_elems[7] * affineOther.m_elems[5] + m_elems[11] * affineOther.m_elems[6] /*+ m_elems[15] * affineOther.m_elems[7]*/,
        m_elems[0] * affineOther.m_elems[8] + m_elems[4] * affineOther.m_elems[9] + m_elems[8] * affineOther.m_elems[10] /*+ m_elems[12] * affineOther.m_elems[11]*/,
        m_elems[1] * affineOther.m_elems[8] + m_elems[5] * affineOther.m_elems[9] + m_elems[9] * affineOther.m_elems[10] /*+ m_elems[13] * affineOther.m_elems[11]*/,
        m_elems[2] * affineOther.m_elems[8] + m_elems[6] * affineOther.m_elems[9] + m_elems[10] * affineOther.m_elems[10] /*+ m_elems[14] * affineOther.m_elems[11]*/,
        0.f,///*m_elems[3] * affineOther.m_elems[8]*/ + m_elems[7] * affineOther.m_elems[9] + m_elems[11] * affineOther.m_elems[10] /*+ m_elems[15] * affineOther.m_elems[11]*/,
        m_elems[0] * affineOther.m_elems[12] + m_elems[4] * affineOther.m_elems[13] + m_elems[8] * affineOther.m_elems[14] + m_elems[12] /** affineOther.m_elems[15]*/,
        m_elems[1] * affineOther.m_elems[12] + m_elems[5] * affineOther.m_elems[13] + m_elems[9] * affineOther.m_elems[14] + m_elems[13] /** affineOther.m_elems[15]*/,
        m_elems[2] * affineOther.m_elems[12] + m_elems[6] * affineOther.m_elems[13] + m_elems[10] * affineOther.m_elems[14] + m_elems[14] /** affineOther.m_elems[15]*/,
        1.f///*m_elems[3] * affineOther.m_elems[12]*/ + m_elems[7] * affineOther.m_elems[13] + m_elems[11] * affineOther.m_elems[14] + m_elems[15] /** affineOther.m_elems[15]*/
        );
}

void
Matrix4::AffineGetTranslation(Vector3& translation) const
{
    translation.x = m_elems[12];
    translation.y = m_elems[13];
    translation.z = m_elems[14];
}

void
Matrix4::AffineGetRotation(Matrix3& rotation) const
{
    rotation = Matrix3(
        m_elems[0], m_elems[4], m_elems[8],
        m_elems[1], m_elems[5], m_elems[9],
        m_elems[2], m_elems[6], m_elems[10]);
}

void
Matrix4::AffineSet()
{
    // set the bottom row to [0 0 0 1]
    m_elems[3] = m_elems[7] = m_elems[11] = 0.0f;
    m_elems[15] = 1.0f;
}

void 
Matrix4::AffineSetTranslation(const Vector3& translation)
{
    m_elems[12] = translation.x;
    m_elems[13] = translation.y;
    m_elems[14] = translation.z;
    m_elems[15] = 1.0f;
}

void 
Matrix4::AffineSetScale(const Vector3& scaleFactor)
{
    m_elems[0] *= scaleFactor.x;
    m_elems[5] *= scaleFactor.y;
    m_elems[10] *= scaleFactor.z;
}

void 
Matrix4::AffineSetUniformScale(float scaleFactor)
{
    m_elems[0] *= scaleFactor;
    m_elems[5] *= scaleFactor;
    m_elems[10] *= scaleFactor;
}

void 
Matrix4::AffineSetRotation(const Matrix3& rotation)
{
    m_elems[0] = rotation.Get(0, 0); m_elems[4] = rotation.Get(0, 1); m_elems[8] = rotation.Get(0, 2);
    m_elems[1] = rotation.Get(1, 0); m_elems[5] = rotation.Get(1, 1); m_elems[9] = rotation.Get(1, 2);
    m_elems[2] = rotation.Get(2, 0); m_elems[6] = rotation.Get(2, 1); m_elems[10] = rotation.Get(2, 2);
}

void 
Matrix4::AffineInverse()
{
    // for affine transforms the inverse can be computed as
    // [ R'  -R'*T ]
    // [ 0      1  ]
    // where R' is the R transpose

    Matrix3 R; AffineGetRotation(R);
    Vector3 T; AffineGetTranslation(T);

    R.Transpose();
    AffineSetRotation(R);
    T.Invert();
    AffineSetTranslation(R.MulVec3Right(T));
    AffineSet();
}

} // namespace hephaestus
