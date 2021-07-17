#ifndef __LAB_SSAO_H__
#define __LAB_SSAO_H__

#include "../Rasterizer/rasterizer.h"
#include "../Shaders/Shaders.h"
#include "../QGL/SSAO.h"
using namespace QGL;

void Lab_SSAO() {

    const int width = 800;
    const int height = 800;
    const int depth = 10;
    const float near = 1.f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(3,2,3);
    const Vec3f center(0,0,0);
    const Vec3f up(0,1,0);

    // 加载模型
    StrangeModel *marry = new StrangeModel("../asset/Marry/Marry.obj");
    marry->loadMap("../asset/Marry/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);
    marry->scale(1, 1, 1);

    StrangeModel *plane = new StrangeModel("../asset/plane/plane.obj");
    Vec4f color(100, 100, 100, 255);
    plane->loadMap(color, QGL::MT_Diffuse);
    plane->scale(1, 1, 1);


    // 
    Frame frame = Frame(width, height, 4);
    Zbuffer zb = Zbuffer(width, height);
    Log log = Log(true, "Shadow Shading: ");
    QGL::Timer timer = QGL::Timer();

    LookAt(camera, center, up);
    SetProjectMat(fov, ratio, near, far);
    SetScreenMat(0, 0, width, height, depth);
    Init();

    DepthShader depthShader = DepthShader();
    depthShader.uniform_mat_obj2view = QGL::MAT_VIEW*QGL::MAT_MODEL;
    depthShader.uniform_mat_transform = QGL::MAT_TRANS;
    depthShader.uniform_max_depth = depth;    


    // Rasterize
    RasterNode rn;
    rn.shader = &depthShader;
    rn.frame = &frame;
    rn.zbuffer = &zb;
    rn.comType = QGL::CT_Single;

    log = Log(true, "Marry Shading: ");
    depthShader.model = marry;
    rn.model = marry;
    rn.log = &log;
    timer.update();
    Rasterize(rn);
    timer.show();


    log = Log(true, "Plane Shading: ");
    depthShader.model = plane;
    rn.model = plane;
    rn.log = &log;
    timer.update();
    Rasterize(rn);
    timer.show();

    Frame depthmap = frame.copy();

    // draw depthmap
    std::string dimg = "../examples/SSAO/depth_2400*2400.png";
    frame.flip();
    frame.draw(dimg.c_str());

    frame.clear();

    // post-process
    timer.update();
    ScreenSpaceAmbientOcclusion(frame, zb);
    timer.show();

    // // draw SSAO image
    std::string img = "../examples/SSAO/SSAO_2400*2400.png";
    frame.flip();
    frame.draw(img.c_str());        

}

#endif