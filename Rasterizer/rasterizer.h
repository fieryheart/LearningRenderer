#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include "../Geometry/geometry.h"

namespace QGL {

// 线程数量
extern int NUM_THREADS;

// 一系列变换矩阵
extern Matrix MAT_MODEL;
extern Matrix MAT_VIEW;
extern Matrix MAT_PERS_PROJECT;
extern Matrix MAT_ORTHO_PROJECT;
extern Matrix MAT_SCREEN;


void LookAt(Vec3f eye, Vec3f center, Vec3f up);
void SetPerspectiveProjectMat(float near, float far);
void SetOrthogonalProjectMat(int l, int r, int b, int t, int n, int f);
void SetOrthogonalProjectMat(int w, int h, int depth);
void SetScreenMat(int x, int y, int w, int h, int depth);


void Init();

}

#endif // __RASTERIZER_H__