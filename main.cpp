#include <iostream>

#include "Rasterizer/rasterizer.h"
#include "Shaders/Shaders.h"

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

    LookAt(camera, center, up);
    SetProjectMat(fov, ratio, near, far);
    SetScreenMat(0, 0, width, height, depth);

    Init();

    // 加载模型
    StrangeModel *marry = new StrangeModel("../asset/Marry/Marry.obj");
    marry->loadMap("../asset/Marry/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);
    marry->scale(1, 1, 1);
    // marry->rotate(0, 0, 0);
    // marry->translate(0, 0, -2);

    StrangeModel *plane = new StrangeModel("../asset/plane/plane.obj");
    Vec4f color(100, 100, 100, 255);
    plane->loadMap(color, QGL::MT_Diffuse);
    plane->scale(1, 1, 1);

    // 
    Frame frame = Frame(width, height, 4);
    Zbuffer zb = Zbuffer(width, height);
    Log log = Log(true, "Tex Shading: ");
    QGL::Timer timer = QGL::Timer();
    

    // Shader
    TexShader texShader = TexShader();
    texShader.uniform_mat_obj2view = QGL::MAT_VIEW*QGL::MAT_MODEL;
    texShader.uniform_mat_transform = QGL::MAT_TRANS;

    // Rasterize
    RasterNode rn;
    rn.shader = &texShader;
    rn.frame = &frame;
    rn.zbuffer = &zb;
    rn.comType = QGL::CT_Single;


    // rasterizer
    log = Log(true, "Marry Shading: ");
    texShader.model = marry;
    rn.model = marry;
    rn.log = &log;

    timer.update();
    Rasterize(rn);
    timer.show();


    log = Log(true, "Plane Shading: ");
    texShader.model = plane;
    rn.model = plane;
    rn.log = &log;

    timer.update();
    Rasterize(rn);
    timer.show();


    // filter
    // std::vector<float> kernel(9, 1/9.f);
    // frame.filter(kernel, 3, 3);

    std::string img = "../examples/TexShader/out_2400*2400.png";
    frame.flip();
    frame.draw(img.c_str());

    delete marry;
    delete plane;
}

// 实现 blinn phong模型
void Lab_Phong() {
    const int width = 2400;
    const int height = 2400;
    const int depth = 200;
    const float near = 1.f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(0,0.5,1);
    const Vec3f center(0,0.5,0);
    const Vec3f up(0,1,0);
    Vec3f light(0,-1,-1);

    LookAt(camera, center, up);
    SetProjectMat(fov, ratio, near, far);
    SetScreenMat(0, 0, width, height, depth);

    Init();

    // 加载模型
    StrangeModel *marry = new StrangeModel("../asset/Marry/Marry.obj");
    marry->loadMap("../asset/Marry/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);
    marry->scale(1, 1, 1);
    marry->rotate(0, -30, 0);
    marry->translate(0, -0.4, -2);

    // 
    Frame frame = Frame(width, height, 4);
    Zbuffer zb = Zbuffer(width, height);
    Log log = Log(true, "Phong Shading: ");
    QGL::Timer timer = QGL::Timer();

    // Shader
    PhongShader phongShader = PhongShader();
    phongShader.model = marry;
    phongShader.uniform_mat_obj2view = QGL::MAT_VIEW*QGL::MAT_MODEL;
    phongShader.uniform_mat_transform = QGL::MAT_TRANS;
    phongShader.uniform_mat_norm_transform = QGL::MAT_NORM_TRANS;
    phongShader.uniform_camera = camera;
    phongShader.uniform_light = light.normalize();

    // Rasterize
    RasterNode rn;
    rn.model = marry;
    rn.shader = &phongShader;
    rn.frame = &frame;
    rn.zbuffer = &zb;
    rn.log = &log;
    rn.comType = QGL::CT_Single;

    timer.update();
    Rasterize(rn);
    timer.show();

    // filter
    // std::vector<float> kernel(9, 1/9.f);
    // frame.filter(kernel, 3, 3);

    std::string img = "../examples/PhongShader/out_800*800.png";
    frame.flip();
    frame.draw(img.c_str());

    delete marry;    
}

// 实现 阴影效果
void Lab_Shadow() {
    const int width = 2400;
    const int height = 2400;
    const int depth = 10;
    const float near = 1.f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(2,2,3);
    const Vec3f center(0,0,0);
    const Vec3f up(0,1,0);
    Vec3f light(-1,3,3);

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

    LookAt(light, center, up);
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

    // rasterizer
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
    std::string dimg = "../examples/ShadowShader/out_depth_2400*2400.png";
    frame.flip();
    frame.draw(dimg.c_str());


    // render shadow
    frame.clear();
    zb.clear();

    LookAt(camera, center, up);
    Init();

    ShadowShader shadowShader = ShadowShader();
    shadowShader.uniform_mat_obj2view = QGL::MAT_VIEW*QGL::MAT_MODEL;
    shadowShader.uniform_mat_depth_transform = depthShader.uniform_mat_transform;
    shadowShader.uniform_mat_depth_obj2view = depthShader.uniform_mat_obj2view;
    shadowShader.uniform_mat_transform = QGL::MAT_TRANS;
    shadowShader.uniform_mat_transform_i = (QGL::MAT_TRANS).inverse();
    shadowShader.uniform_mat_norm_transform = MAT_NORM_TRANS;
    shadowShader.uniform_camera = camera;
    shadowShader.uniform_light = light;
    shadowShader.uniform_max_depth = depth;
    shadowShader.depthmap = &depthmap;

    rn.shader = &shadowShader;

    // rasterizer
    log = Log(true, "Marry Shading: ");
    shadowShader.model = marry;
    rn.model = marry;
    rn.log = &log;
    timer.update();
    Rasterize(rn);
    timer.show();


    log = Log(true, "Plane Shading: ");
    shadowShader.model = plane;
    rn.model = plane;
    rn.log = &log;
    timer.update();
    Rasterize(rn);
    timer.show();

    // draw shadow
    std::string img = "../examples/ShadowShader/out_2400*2400.png";
    frame.flip();
    frame.draw(img.c_str());

    delete marry; 
    delete plane;
}

int main()
{
    // Lab_Tex();
    // Lab_Phong();
    Lab_Shadow();
    return 0;
}