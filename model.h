#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> texCoord_;
	std::vector<Vec3f> norm_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> facesTex_;
	std::vector<std::vector<int>> facesNorm_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int ntexCoords();
	int nfaces();
	std::vector<int> face(int idx);
	std::vector<int> faceTex(int idx);
	Vec4f vert(int nthface, int nthvert);
	Vec3f texCoord(int nthface, int nthvert);
	Vec3f norm(int i, int j);
	Vec3f normal(Vec3f uv);
	TGAColor diffuse(Vec3f uv);
	float specular(Vec3f uv);
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
};

#endif //__MODEL_H__