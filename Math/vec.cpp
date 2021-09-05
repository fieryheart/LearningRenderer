#include "vec.hpp"

namespace QGL {

template <> template <> Vec3<int>::Vec3(const Vec3<float> &v) : x(v.x+.5f), y(v.y+.5f), z(v.z+.5f) {}
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v) : x(v.x), y(v.y), z(v.z) {};

}