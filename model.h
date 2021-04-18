#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> texCoord_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > facesTex_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int ntexCoords();
	int nfaces();
	std::vector<int> face(int idx);
	std::vector<int> faceTex(int idx);
	Vec3f vert(int i);
	Vec3f texCoord(int idx);
};

#endif //__MODEL_H__