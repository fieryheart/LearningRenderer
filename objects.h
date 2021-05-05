#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include "geometry.h"

namespace QGL {
// 
class Object {
public:
    virtual ~Object() {};
};

// 面：对象坐标、法向量、纹理坐标
struct Face {
    Vec3i v;
    Vec3i vn;
    Vec3i vt;
    Face(){}
    Face(Vec3i v) : v(v){}
    Face(Vec3i v, Vec3i vn) : v(v), vn(vn){}
    Face(Vec3i v, Vec3i vn, Vec3i vt) : v(v), vn(vn), vt(vt){}
};

// 三角形：点、法向量
class Triangle : public Object {
private:
    Vec3f points[3];
    Vec3f normal;
public:
    Triangle() {};
    ~Triangle() {};
    Triangle(Vec3f *_points) {points[0] = _points[0]; points[1] = _points[1]; points[2] = _points[2];}
    Triangle(Vec3f *_points, Vec3f _normal) {points[0] = _points[0]; points[1] = _points[1]; points[2] = _points[2]; normal = _normal;}
    Vec3f getPoints(int index) {
        if (index >= 0 && index <= 2) throw "Index is more than 2.";
        return points[index];
    }
    void setPoints(int index, Vec3f point) {
        if (index >= 0 && index <= 2) throw "Index is more than 2.";
        points[index] = point;
    }
    Vec3f getNormal() {
        return normal;
    }
    void setNormal(Vec3f _normal) {
        normal = _normal;
    }
};

// 四边形： 点、法向量
// class Rectangle : public Object {
// private:

// public:
// };
}
#endif // __OBJECTS_H__