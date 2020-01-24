#include <common/Vector4.h>

namespace hephaestus
{

const Vector4 Vector4::ZERO = Vector4(0.f, 0.f, 0.f, 0.f);
const Vector4 Vector4::UNARY = Vector4(1.f, 1.f, 1.f, 1.f);
const Vector4 Vector4::UNIT_X = Vector4(1.f, 0.f, 0.f, 0.f);
const Vector4 Vector4::UNIT_Y = Vector4(0.f, 1.f, 0.f, 0.f);
const Vector4 Vector4::UNIT_Z = Vector4(0.f, 0.f, 1.f, 0.f);
const Vector4 Vector4::UNIT_W = Vector4(0.f, 0.f, 0.f, 1.f);
const Vector4 Vector4::NEG_UNIT_X = Vector4(-1.f, 0.f, 0.f, 0.f);
const Vector4 Vector4::NEG_UNIT_Y = Vector4(0.f, -1.f, 0.f, 0.f);
const Vector4 Vector4::NEG_UNIT_Z = Vector4(0.f, 0.f, -1.f, 0.f);
const Vector4 Vector4::NEG_UNIT_W = Vector4(0.f, 0.f, 0.f, -1.f);

} // hephaestus
