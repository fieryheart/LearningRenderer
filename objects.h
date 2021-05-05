#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include "geometry.h"

// 
class Object {
public:
    virtual ~Object() {};
};

// 点：对象坐标、法向量、纹理坐标
class Point : public Object {
private:
    Vec3f coordinate;
    Vec3f normal;
    Vec2f uv;
public:
    Point(){}
    ~Point(){}
    Point(Vec3f &p) : coordinate(p) {}
    Point(Vec3f &p, Vec3f &n) : coordinate(p), normal(n) {}
    Point(Vec3f &p, Vec3f &n, Vec2f &u) : coordinate(p), normal(n), uv(u) {}
    Point(Vec3f p) : coordinate(p) {}
    Point(Vec3f p, Vec3f n) : coordinate(p), normal(n) {}
    Point(Vec3f p, Vec3f n, Vec2f u) : coordinate(p), normal(n), uv(u) {}

    // get && set
    Vec3f getCoordinate() {return coordinate;}
    Vec3f getNormal() {return normal;}
    Vec2f getUV() {return uv;}
    void setCoordinate(Vec3f &p) {coordinate = p;}
    void setNormal(Vec3f &n) {normal = n;}
    void setUV(Vec2f &u) {uv = u;}
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

#endif // __OBJECTS_H__