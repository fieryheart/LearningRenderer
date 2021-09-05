#ifndef __TESTRASTER_H__
#define __TESTRASTER_H__

#include "../Rasterizer/rasterizer.hpp"
#include "../Shaders/Shaders.hpp"
using namespace QGL;

// 纹理贴图
void TestTexture() {
    const int width = 800;
    const int height = 800;
    const int depth = 255;
    const float near = 0.01f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(4,4,5);
    const Vec3f center(0,1.5,0);
    const Vec3f up(0,1,0);

    std::cout << "===========TestTexture===========" << endl;

    // 创建变换矩阵
    Vec3f scale = {1.0, 1.0, 1.0};
    Vec3f rotate = {1.0, 1.0, 1.0};
    Vec3f translate = {0.0, 0.0, 0.0};
    Matrix model_mat = GenModelMat(scale, rotate, translate);
    Matrix view_mat = GenViewMat(camera, center, up);
    Matrix persp_mat = GenPerspMat(fov, ratio, near, far);
    Matrix viewport_mat = GenViewportMat(0, 0, width, height, depth);
    Matrix camera_mat = viewport_mat*persp_mat*view_mat*model_mat;

    // 创建模型
    Object *marry = new StrangeObject("../asset/Marry/Marry.obj");
    Object *plane = new StrangeObject("../asset/plane/plane.obj");

    // 创建光源
    Light *directLight0 = new DirectLight(Vec3f(-1.0, -1.0, -1.0));

    // shaders
    /// 仅加载纹理
    TexShader texShader = TexShader();
    texShader.uniform_mat_world2view = view_mat*model_mat;
    texShader.uniform_mat_transform = camera_mat;  
    texShader.texture0("../asset/Marry/MC003_Kozakura_Mari.png");
    
    Rasterizer rasterizer;
    rasterizer.model = marry;
    // rasterizer.model = plane;
    rasterizer.frame = Frame(width, height, 4);
    rasterizer.zbuffer = Zbuffer(width, height);
    rasterizer.log = Log(false, "Tex Shading: ");
    rasterizer.lights.push_back(directLight0);

    rasterizer.shadow(model_mat);

    // 渲染 marry
    // rasterizer.shader = &texShader;
    // rasterizer.render();

    // 渲染 plane
    // rasterizer.model = plane;
    // texShader.texture0(Vec4f(100, 100, 100, 255));
    // rasterizer.render();

    // rasterizer.draw("../examples/TexShader/marry_800*800.png");
    directLight0->draw("../examples/TexShader/directLight0_800*800.png");

    delete marry;
    delete plane;
    delete directLight0;
}

#endif