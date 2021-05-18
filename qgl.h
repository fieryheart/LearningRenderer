#ifndef __QGL_H__
#define __QGL_H__

#include "model.h"
#include "shaders.h"
#include "RayTracing.h"

namespace QGL {
struct RenderNode {
    Model *model;
    Shader *shader;
    Frame *frame;
    Zbuffer *zbuffer;
    // float *zbuffer;
    ComType comType;
    Log *log;
    RenderNode() {}
};

struct RenderPTNode {
    Frame *frame;
    std::vector<Model*> models;
    Vec3f camera;
    int width;
    int height;
    float fov;
    int bound;
    // int light;
    RenderPTNode(){}
};

extern int NUMTHREADS;

extern Matrix MAT_MODEL;   // 模型空间
extern Matrix MAT_VIEW;    // 相机空间
extern Matrix MAT_PPROJECT; // 透视投影空间
extern Matrix MAT_OPROJECT; // 正交投影空间
extern Matrix MAT_SCREEN;  // 屏幕空间
extern Matrix MAT_TRANS;
extern Matrix MAT_NORM_TRANS;
extern Matrix MAT_NORM_IT;

extern Vec4f BACKGROUND_COLOR;

void SetModelMat();
void SetViewMat(Vec3f eye, Vec3f center, Vec3f up);
void LookAt(Vec3f eye, Vec3f center, Vec3f up);  // the same as SetViewMat.
void SetPerspectiveProjectMat(Vec3f camera, Vec3f origin);
void SetOrthogonalProjectMat(int l, int r, int b, int t, int n, int f);
void SetOrthogonalProjectMat(int w, int h, int depth);
void SetScreenMat(int x, int y, int w, int h, int depth);

// 设置相机
void SetCamera(bool isPercent);

Vec3f barycentric(Vec4f *pts, Vec2f P);

// 模型光栅化
void Rendering(RenderNode &rn);
void SingleRendering(RenderNode &rn);
void OmpRendering(RenderNode &rn);

void DrawTriangle(Vec4f *points, RenderNode &rn);


// 路径追踪
void RenderingByPathTracing(RenderPTNode &in);
Vec4f RayTracing(BVHBuilder *bvh, Ray &ray, int index, Vec3f bc, Vec3f p);
}





// Matrix GetModelMatrix();
// Matrix GetViewMatrix(Vec3f eye, Vec3f center, Vec3f up);
// Matrix GetProjectMatrix(Vec3f camera, Vec3f origin);
// Matrix GetViewportMatrix(int x, int y, int w, int h, int depth);



// void triangle(Vec4f *pts, Shader &shader, TGAImage &image, float *zbuffer);

// void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);

#endif // __QGL_H__