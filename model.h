#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <random>
#include "objects.h"

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
	virtual int nverts();
	virtual int nfaces();
	Face face(int nthface);
	virtual Vec3f vert(int nthface, int nthvert);
	virtual Vec3f norm(int nthface, int nthvert);
	Vec2f tex(int nthface, int nthvert);
	void preprocess();	// 顶点数据映射至[-1, 1]
	void loadMap(const char *filename, MapType mt);
	void sampleDiffuse(Vec2f uv, Vec4f &color);
	virtual Vec3f randomRay(int nthface);
};

// // 四边形： 点、法向量
// struct Plane : Object {
//     Vec3f points[4];
//     Vec3f normal;
//     Vec4f colors[4];
//     float emission;
//     Plane() {}
//     Plane(Vec3f *pts, Vec4f &color_) {
//         points[0] = pts[0];
//         points[1] = pts[1];
//         points[2] = pts[2];
//         points[3] = pts[3];

//         Vec3f ab = points[1] - points[0];
//         Vec3f ac = points[2] - points[0];
//         normal = (ac^ab).normalize();

//         colors[0] = color_;
//         colors[1] = color_;
//         colors[2] = color_;
//         colors[3] = color_;
//     }
//     ~Plane() {}
//     bool interact(Ray &ray, float &t) {
//         // todo
//         return true;
//     }
// };

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
	float emit();
	float pdf();
};
/*
// 模型: 面 + 纹理A + 纹理B + ...
// 面:  点ABC(对象坐标、法向量、纹理坐标)
// 贴图: 
class Model {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> textures;
	std::vector<Face> faces;
	std::vector<Vec4f> colors;
	Sample2D *diffusemap_;
	// Sample2D *normalmap_;
	// Sample2D *specularmap_;

	std::vector<float> emissions;
	float area;

	std::mt19937 rng;
	std::uniform_real_distribution<float> distribution;
public:
	Model(const char *filename);
	Model(Plane plane);
	Model(std::vector<Vec3f> &_verts, std::vector<Face> &_faces);
	~Model();

	int nverts();
	int nfaces();
	Face face(int nthface);
	Vec3f vert(int nthface, int nthvert);
	Vec3f norm(int nthface, int nthvert);
	Vec2f tex(int nthface, int nthvert);
	void vertsNormalize();	// 顶点数据映射至[-1, 1]
	void loadMap(const char *filename, MapType mt);
	// void sampleDiffuse(Vec2i uv, Vec4f &color);
	void sampleDiffuse(Vec2f uv, Vec4f &color);
	void sampleDiffuse(int nthface, Vec3f bc, Vec4f &color);
	Vec3f randomSample();
	Vec3f randomSampleInP();
	float emit();
	float pdf();
};
*/
}

// class Model {
// private:
// 	std::vector<Vec3f> verts_;
// 	std::vector<Vec3f> texCoord_;
// 	std::vector<Vec3f> norm_;
// 	std::vector<std::vector<int>> faces_;
// 	std::vector<std::vector<int>> facesTex_;
// 	std::vector<std::vector<int>> facesNorm_;
// 	TGAImage diffusemap_;
// 	TGAImage normalmap_;
// 	TGAImage specularmap_;
// public:
// 	Model(const char *filename);
// 	~Model();
// 	int nverts();
// 	int ntexCoords();
// 	int nfaces();
// 	std::vector<int> face(int idx);
// 	std::vector<int> faceTex(int idx);
// 	Vec4f vert(int nthface, int nthvert);
// 	Vec3f texCoord(int nthface, int nthvert);
// 	Vec3f norm(int i, int j);
// 	Vec3f normal(Vec3f uv);
// 	TGAColor diffuse(Vec3f uv);
// 	float specular(Vec3f uv);
// 	void load_texture(std::string filename, const char *suffix, TGAImage &img);

// 	int getDiffuseMapWidth();
// 	int getDiffuseMapHeight();
// 	// void generate_normal_mapping(const char *filename);
// };

#endif //__MODEL_H__