#ifndef __VEC_H__
#define __VEC_H__

#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>

namespace QGL {
template<class T> 
struct Vec2 {
    T x, y;
    Vec2<T>() : x(T()), y(T()) {}
    Vec2<T>(T _x) : x(_x), y(_x) {}
    Vec2<T>(T _x, T _y) : x(_x), y(_y) {}
    Vec2<T>(const Vec2<T>& v) : x(v.x), y(v.y) {
        // std::cout << "Vec2 拷贝构造" << std::endl;
    }
    Vec2<T>(const Vec2<T>&& v) : x(v.x), y(v.y) {
        // std::cout << "Vec2 移动构造" << std::endl;
    }
    Vec2<T>& operator=(const Vec2<T>& v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }
    Vec2<T>& operator=(const Vec2<T>&& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    // 加法
    Vec2<T> operator+(const Vec2<T>& V) const { return Vec2<T>(x+V.x, y+V.y); }
    Vec2<T> operator+(Vec2<T>&& V) const { return Vec2<T>(x+V.x, y+V.y); }

    // 减法
    Vec2<T> operator-(const Vec2<T>& V) const { return Vec2<T>(x-V.x, y-V.y); }
    Vec2<T> operator-(Vec2<T>&& V) const { return Vec2<T>(x-V.x, y-V.y); }

    // 乘法
    Vec2<T> operator*(const Vec2<T>& V) const { return Vec2<T>(x*V.x, y*V.y); }
    Vec2<T> operator*(Vec2<T>&& V) const { return Vec2<T>(x*V.x, y*V.y); }
    Vec2<T> operator*(float f) const { return Vec2<T>(x*f, y*f); }

    float norm() const { return std::sqrt(x*x+y*y); }
    T& operator[](const int i) { if (i<=0) return x; else return y; }

    // 输出
    template <class T1> friend std::ostream& operator<<(std::ostream& s, Vec2<T1>&& v);
    template <class T1> friend std::ostream& operator<<(std::ostream& s, Vec2<T1>& v);
};

template <class t> struct Vec3 {
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) { }
    Vec3<t>(t _t) : x(_t), y(_t), z(_t) {}
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class u> Vec3<t>(const Vec3<u> &v);
    Vec3<t>(const Vec3<t> &v) : x(t()), y(t()), z(t()) { *this = v; }
    Vec3<t> & operator =(const Vec3<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }
        return *this;
    }
    // Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    Vec3<t> operator +(const t v) const { return Vec3<t>(x+v, y+v, z+v); }
    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    Vec3<t> operator -(const t v) const { return Vec3<t>(x-v, y-v, z-v); }
    Vec3<t> operator -() const { return Vec3<t>(-x, -y, -z); }

    Vec3<t> operator *(const Vec3<t>& v) const { return Vec3<t>(x*v.x, y*v.y, z*v.z); }
    Vec3<t> operator *(const Vec3<t>&& v) const { return Vec3<t>(x*v.x, y*v.y, z*v.z); }
    Vec3<t> operator *(float f) const { return Vec3<t>(x*f, y*f, z*f); }
    
    Vec3<t> operator /(float f)          const { return Vec3<t>(x/f, y/f, z/f); }

    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    float norm2 () const { return x*x+y*y+z*z; }
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    t& operator[](const int i) { if (i<=0) return x; else if (i==1) return y; else return z; }

    template <class T1> friend std::ostream& operator<<(std::ostream& s, Vec3<T1>&& v);
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};


template <class t> struct Vec4 {
    t x, y, z, w;
    Vec4<t>() : x(t()), y(t()), z(t()), w(t()) { }
    Vec4<t>(t _t) : x(_t), y(_t), z(_t), w(_t) {}
    Vec4<t>(t _x, t _y, t _z, t _w) : x(_x), y(_y), z(_z), w(_w) {}
    template <class u> Vec4<t>(const Vec3<u> &v, t _w) : x(v.x), y(v.y), z(v.z), w(_w) {}
    // template <class u> Vec4<t>(const Vec4<u> &v);
    Vec4<t>(const Vec4<t> &v) : x(t()), y(t()), z(t()), w(t()) { *this = v; }
    Vec4<t> & operator =(const Vec4<t> &v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
            w = v.w;
        }
        return *this;
    }
    Vec4<t> operator +(const Vec4<t> &v) const { return Vec4<t>(x+v.x, y+v.y, z+v.z, w+v.w); }
    Vec4<t> operator -(const Vec4<t> &v) const { return Vec4<t>(x-v.x, y-v.y, z-v.z, w-v.w); }
    Vec4<t> operator *(float f)          const { return Vec4<t>(x*f, y*f, z*f, w*f); }
    Vec4<t> operator /(float f)          const { return Vec4<t>(x/f, y/f, z/f, w/f); }
    // t       operator *(const Vec4<t> &v) const { return x*v.x + y*v.y + z*v.z + w*v.w; }
    Vec4<t>       operator *(const Vec4<t> &v) const { return Vec4<t>(x*v.x, y*v.y, z*v.z, w*v.w); }
    float norm () const { return std::sqrt(x*x+y*y+z*z+w*w); }
    Vec4<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    t& operator[](const int i) { if (i<=0) return x; else if (i==1) return y; else if (i==2) return z; else return w; }
    Vec3<t> v3f() { return Vec3<float>((float)x, (float)y, (float)z); }


    template <class T1> friend std::ostream& operator<<(std::ostream& s, Vec4<T1>&& v);
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec4<t>& v);
};


// 输出
template <class T> 
std::ostream& operator<<(std::ostream& s, Vec2<T>&& v) {
    s << "(" << v.x << ", " << v.y << ")";
    return s;
}
template <class T> 
std::ostream& operator<<(std::ostream& s, Vec2<T>& v) {
    s << "(" << v.x << ", " << v.y << ")";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>&& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return s;
}
template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec4<t>&& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec4<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return s;
}

// 类型转换
template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);


typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
typedef Vec4<float> Vec4f;
}


// template <> template <> Vec4<int>::Vec4(const Vec4<float> &v);
// template <> template <> Vec4<float>::Vec4(const Vec4<int> &v);
#endif //__VEC_H__