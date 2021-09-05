#include "calculate.hpp"

namespace QGL {

// 向量
/// 点乘
// float dot(Vec2f &v1, Vec3f &v2) {
//     return v1.x*v2.x + v1.y*v2.y;
// }
float dot(Vec3f &v1, Vec3f &v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
// float dot(Vec3f &v1, Vec3f &v2) {
//     return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
// }

/// 叉乘
Vec3f cross(Vec3f& v1, Vec3f& v2) {
    return Vec3f(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x);
}
Vec3f cross(Vec3f&& v1, Vec3f&& v2) {
    return Vec3f(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x);
}


/// pow
Vec4f pow(Vec4f& v, float f) {
    Vec4f out(0,0,0,1);
    for (int i = 0; i < 3; ++i) {
        out[i] = std::pow(v[i], f);
    }
    return out;
}

// 矩阵


}