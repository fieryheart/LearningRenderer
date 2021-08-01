#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <random>
#include "../Vec/vec.h"
#include "../QGL/types.h"
#include "../QGL/pieces.h"

namespace QGL {

class Model {
public:
	virtual ModelType type() = 0;
	virtual int nverts() = 0;
	virtual int nfaces() = 0;
	virtual Vec3f vert(int nthface, int nthvert) = 0;
	virtual Vec3f norm(int nthface, int nthvert) = 0;
	// virtual float brdf(Vec3f &p, Vec3f &wi, Vec3f &wo) = 0;

	virtual Vec3f randomRay(int nthface) = 0;
};

class StrangeModel : public Model {
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

	Sample2D *diffusemap_;
	// Sample2D *normalmap_;
	// Sample2D *specularmap_;

	std::mt19937 rng;
	std::uniform_real_distribution<float> distribution;	
public:
	StrangeModel(const char *filename);
	~StrangeModel();
	virtual ModelType type();
	float brdf(int nthface, Vec3f p, Vec3f wi, Vec3f wo);
	virtual int nverts();
	virtual int nfaces();
	Face face(int nthface);
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
	Vec3f norm(int nthface, Vec3f bc);
	Vec2f tex(int nthface, int nthvert);
	void preprocess();	// 顶点数据映射至[-1, 1]
	void loadMap(const char *filename, MapType mt);
	void loadMap(Vec4f color, MapType mt);
	void sampleDiffuse(Vec2f uv, Vec4f &color);
	void sampleDiffuse(int nthface, Vec3f bc, Vec4f &color);
	virtual Vec3f randomRay(int nthface);
	void scale(float width, float height, float depth);
	void translate(float x, float y, float z);
	void rotate(float x, float y, float z);
};

class BuildinModel : public Model {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec4f> colors;
	std::vector<Vec3i> faces;
	std::vector<MatrialType> mt_types;

	std::mt19937 rng;
	std::uniform_real_distribution<float> distribution;	
public:
	BuildinModel(std::vector<Vec3f> &verts_, 
				 std::vector<Vec3f> &normals_,
				 std::vector<Vec4f> &colors_,
				 std::vector<Vec3i> &faces_,
				 std::vector<MatrialType> &mt_types_);
	~BuildinModel(){}
	virtual ModelType type();
	float brdf(int nthface, Vec3f p, Vec3f wi, Vec3f wo);
	virtual int nverts();
	virtual int nfaces();
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
	void sampleDiffuse(int nthface, Vec4f &color);
	virtual Vec3f randomRay(int nthface);
};

// 暂定为平面
class LightModel : public Model {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec3i> faces;
	Vec4f color;
	float intensity;
	float area;

	std::mt19937 rng;
	std::uniform_real_distribution<float> distribution;	
public:
	LightModel(	std::vector<Vec3f> &verts_, 
			   	std::vector<Vec3f> &normals_,
			    std::vector<Vec3i> &faces_,
				Vec4f &color_,
				float intensity_);
	~LightModel(){}
	virtual ModelType type();
	virtual int nverts();
	virtual int nfaces();
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
	Vec3f randomSample();
	virtual Vec3f randomRay(int nthface);
	Vec4f getColor();
	Vec4f emit();
	float pdf();
};
}

#endif //__MODEL_H__