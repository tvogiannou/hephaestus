#include <common/Vector3.h>

namespace hephaestus
{

const Vector3 Vector3::ZERO = Vector3(0.f, 0.f, 0.f);
const Vector3 Vector3::UNARY = Vector3(1.f, 1.f, 1.f);
const Vector3 Vector3::UNIT_X = Vector3(1.f, 0.f, 0.f);
const Vector3 Vector3::UNIT_Y = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::UNIT_Z = Vector3(0.f, 0.f, 1.f);
const Vector3 Vector3::NEG_UNIT_X = Vector3(-1.f, 0.f, 0.f);
const Vector3 Vector3::NEG_UNIT_Y = Vector3(0.f, -1.f, 0.f);
const Vector3 Vector3::NEG_UNIT_Z = Vector3(0.f, 0.f, -1.f);

} // hephaestus
