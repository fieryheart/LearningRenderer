#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "../Geometry/geometry.h"
#include "../QGL/pieces.h"
#include "../QGL/types.h"
#include "../Shaders/shader.h"

namespace QGL {

// 线程数量
extern int NUM_THREADS;

// 一系列变换矩阵
extern Matrix MAT_MODEL;
extern Matrix MAT_VIEW;
extern Matrix MAT_PROJECT;
extern Matrix MAT_ORTHO_PROJECT;
extern Matrix MAT_SCREEN;
extern Matrix MAT_TRANS;
extern Matrix MAT_NORM_TRANS;

struct RasterNode {
    Model *model;
    Shader *shader;
    Frame *frame;
    Zbuffer *zbuffer;
    ComType comType;
    Log *log;
    RasterNode() {}
};


void LookAt(Vec3f eye, Vec3f center, Vec3f up);
void SetProjectMat(float fov, float ratio, float near, float far);
void SetScreenMat(int x, int y, int w, int h, int depth);

void SetPerspectiveProjectMat(Vec3f camera, Vec3f origin);

void Init();

Vec3f barycentric(Vec4f *pts, Vec2f P);
void Rasterize(RasterNode &rn);
void SingleRasterize(RasterNode &rn);
void MultiRasterize(RasterNode &rn);

}

#endif // __RASTERIZER_H__