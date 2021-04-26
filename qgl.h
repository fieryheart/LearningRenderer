#ifndef __QGL_H__
#define __QGL_H__

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

// extern Matrix ModelMatrix;          // 模型空间
// extern Matrix ViewMatrix;           // 视角空间
// extern Matrix ProjectMatrix;        // 投影空间
// extern Matrix ViewportMatrix;       // 屏幕空间

Matrix GetModelMatrix();
Matrix GetViewMatrix(Vec3f eye, Vec3f center, Vec3f up);
Matrix GetProjectMatrix(Vec3f camera, Vec3f origin);
Matrix GetViewportMatrix(int x, int y, int w, int h, int depth);

class Shader {
public:
    virtual ~Shader();
    virtual Vec4f vertex(int nthface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, Shader &shader, TGAImage &image, float *zbuffer);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

#endif // __QGL_H__