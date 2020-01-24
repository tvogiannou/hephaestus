#pragma once

#include <common/Vector3.h>
#include <common/Matrix3.h>

#include <cfloat>


namespace hephaestus
{

struct AxisAlignedBoundingBox
{
	hephaestus::Vector3 min;
	hephaestus::Vector3 max;

    // create/merge
    void Reset()
    {
        min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }
    void AddPoint(const Vector3& p);
	void AddAxisAlignedBoundinBox(const AxisAlignedBoundingBox& bbox);

    // manipulate
    void ApplyTransform(const Matrix3& rotation, const Vector3& translation);

    // get info
	Vector3 ComputeExtends() const;	// return half extends for each axis
	Vector3 ComputeCenter() const;	// return the center of the box

    // query
    bool Contains(const Vector3& p) const;
};

}