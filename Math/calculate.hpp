#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include "vec.hpp"
#include "matrix.hpp"

namespace QGL {

// 向量
/// 点乘
float dot(Vec3f &v1, Vec3f &v2);
/// 叉乘
Vec3f cross(Vec3f& v1, Vec3f& v2);
Vec3f cross(Vec3f&& v1, Vec3f&& v2);
/// pow
Vec4f pow(Vec4f& v, float f);

// 矩阵

}

#endif