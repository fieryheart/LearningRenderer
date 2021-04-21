#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> texCoord_;
	std::vector<Vec3f> norm_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> facesTex_;
	std::vector<std::vector<int>> facesNorm_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int ntexCoords();
	int nfaces();
	std::vector<int> face(int idx);
	std::vector<int> faceTex(int idx);
	Vec3f vert(int nthface, int nthvert);
	Vec3f texCoord(int idx);
	Vec3f norm(int i, int j);
};

#endif //__MODEL_H__