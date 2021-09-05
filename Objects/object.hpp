#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <vector>
#include <random>
#include "../Math/math_init.hpp"
#include "../QGL/types.h"
#include "../QGL/pieces.hpp"
#include "lights.hpp"

namespace QGL {

class Object {
public:
	virtual ObjectType type() {};
	virtual int nverts() {};
	virtual int nfaces() {};
	virtual Vec3f vert(int nthface, int nthvert) {};
	virtual Vec3f norm(int nthface, int nthvert) {};
	virtual Vec2f tex(int nthface, int nthvert) {};
};

class StrangeObject : public Object {
private:
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

	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> textures;
	std::vector<Face> faces;
	MatrialType mtt;
public:
	StrangeObject(const char *filename);
	~StrangeObject(){};
	virtual ObjectType type();
	virtual int nverts();
	virtual int nfaces();
	Face face(int nthface);
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
	virtual Vec2f tex(int nthface, int nthvert);
};

class BuildinObject : public Object {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec4f> colors;
	std::vector<Vec3i> faces;
	std::vector<MatrialType> mt_types;

	std::mt19937 rng;
	std::uniform_real_distribution<float> distribution;	
public:
	BuildinObject(std::vector<Vec3f> &verts_, 
				 std::vector<Vec3f> &normals_,
				 std::vector<Vec4f> &colors_,
				 std::vector<Vec3i> &faces_,
				 std::vector<MatrialType> &mt_types_);
	~BuildinObject(){}
	virtual ObjectType type();
	virtual int nverts();
	virtual int nfaces();
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
};
}

#endif //__Object_H__