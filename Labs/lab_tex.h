#ifndef __LABTEX_H__
#define __LABTEX_H__

#include "../Rasterizer/rasterizer.hpp"
#include "../Shaders/Shaders.hpp"
using namespace QGL;

// 只加载纹理
void Lab_Tex() {
    const int width = 2400;
    const int height = 2400;
    const int depth = 255;
    const float near = 1.f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(2,2,3);
    const Vec3f center(0,0,0);
    const Vec3f up(0,1,0);

    // LookAt(camera, center, up);
    // SetProjectMat(fov, ratio, near, far);
    // SetScreenMat(0, 0, width, height, depth);

    // Init();

    // // 加载模型
    // StrangeModel *marry = new StrangeModel("../asset/Marry/Marry.obj");
    // marry->loadMap("../asset/Marry/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);
    // marry->scale(1, 1, 1);
    // // marry->rotate(0, 0, 0);
    // // marry->translate(0, 0, -2);

    // StrangeModel *plane = new StrangeModel("../asset/plane/plane.obj");
    // Vec4f color(100, 100, 100, 255);
    // plane->loadMap(color, QGL::MT_Diffuse);
    // plane->scale(1, 1, 1);

    // // 
    // Frame frame = Frame(width, height, 4);
    // Zbuffer zb = Zbuffer(width, height);
    // Log log = Log(true, "Tex Shading: ");
    // QGL::Timer timer = QGL::Timer();
    

    // // Shader
    // TexShader texShader = TexShader();
    // texShader.uniform_mat_obj2view = QGL::MAT_VIEW*QGL::MAT_MODEL;
    // texShader.uniform_mat_transform = QGL::MAT_TRANS;

    // // Rasterize
    // RasterNode rn;
    // rn.shader = &texShader;
    // rn.frame = &frame;
    // rn.zbuffer = &zb;
    // rn.comType = QGL::CT_Single;


    // // rasterizer
    // log = Log(true, "Marry Shading: ");
    // texShader.model = marry;
    // rn.model = marry;
    // rn.log = &log;

    // timer.update();
    // Rasterize(rn);
    // timer.show();


    // log = Log(true, "Plane Shading: ");
    // texShader.model = plane;
    // rn.model = plane;
    // rn.log = &log;

    // timer.update();
    // Rasterize(rn);
    // timer.show();


    // // filter
    // // std::vector<float> kernel(9, 1/9.f);
    // // frame.filter(kernel, 3, 3);

    // std::string img = "../examples/TexShader/out_2400*2400.png";
    // frame.flip();
    // frame.draw(img.c_str());

    // delete marry;
    // delete plane;
}

#endif // __LAB_TEX_H__