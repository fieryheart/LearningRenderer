#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "objects.h"

namespace QGL {
// 模型: 面 + 纹理A + 纹理B + ...
// 面:  点ABC(对象坐标、法向量、纹理坐标)
// 贴图: 
class Model {
private:
	std::vector<Vec3f> verts;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> textures;
	std::vector<Face> faces;
	Sample2D *diffusemap_;
	// Sample2D *normalmap_;
	// Sample2D *specularmap_;
public:
	Model(const char *filename);
	Model(Plane plane);
	Model(std::vector<Vec3f> &_verts, std::vector<Face> &_faces);
	~Model();

	int nverts();
	int nfaces();
	Vec3f vert(int nthface, int nthvert);
	Vec3f norm(int nthface, int nthvert);
	Vec2f tex(int nthface, int nthvert);
	void vertsNormalize();	// 顶点数据映射至[-1, 1]
	void loadMap(const char *filename, MapType mt);
	void sampleDiffuse(Vec2i uv, Vec4f &color);
	void sampleDiffuse(Vec2f uv, Vec4f &color);
};
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