#ifndef __QGL_H__
#define __QGL_H__

#include "model.h"
#include "shaders.h"

namespace QGL {
struct Frame {
    int width;
    int height;
    std::vector<float> buffer;
    Frame(int w, int h) : width(h), height(h) {
        buffer = std::vector<float>(w*h, 0.0f);
    }
};

extern Matrix MAT_MODEL;   // 模型空间
extern Matrix MAT_VIEW;    // 相机空间
extern Matrix MAT_PPROJECT; // 透视投影空间
extern Matrix MAT_OPROJECT; // 正交投影空间
extern Matrix MAT_SCREEN;  // 屏幕空间

void SetModelMat();
void SetViewMat(Vec3f eye, Vec3f center, Vec3f up);
void LookAt(Vec3f eye, Vec3f center, Vec3f up);  // the same as SetViewMat.
void SetPerspectiveProjectMat(Vec3f camera, Vec3f origin);
void SetOrthogonalProjectMat();
void SetScreenMat(int x, int y, int w, int h, int depth);

// 设置相机
void SetCamera();

// 模型光栅化
void Rasterizer(Model *model);

void DrawTriangle(Vec4f *points, Shader &shader);
}





// Matrix GetModelMatrix();
// Matrix GetViewMatrix(Vec3f eye, Vec3f center, Vec3f up);
// Matrix GetProjectMatrix(Vec3f camera, Vec3f origin);
// Matrix GetViewportMatrix(int x, int y, int w, int h, int depth);



// void triangle(Vec4f *pts, Shader &shader, TGAImage &image, float *zbuffer);

// void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

#endif // __QGL_H__