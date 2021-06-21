#include <iostream>

#include "Rasterizer/rasterizer.h"
#include "Shaders/Shaders.h"

using namespace QGL;

void Lab_0() {
    const int width = 800;
    const int height = 800;
    const int depth = 200;
    const float near = 1.f;
    const float far = 100.f;
    const float fov = 45.f;
    const float ratio = 1.f;

    const Vec3f camera(0,0.5,1);
    const Vec3f center(0,0.5,0);
    const Vec3f up(0,1,0);

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
    Log log = Log(true, "Tex Shading: ");
    QGL::Timer timer = QGL::Timer();

    // Shader
    TexShader texShader = TexShader();
    texShader.model = marry;
    texShader.uniform_mat_transform = QGL::MAT_TRANS;

    // Rasterize
    RasterNode rn;
    rn.model = marry;
    rn.shader = &texShader;
    rn.frame = &frame;
    rn.zbuffer = &zb;
    rn.log = &log;
    rn.comType = QGL::CT_Single;

    timer.update();
    Rasterize(rn);
    timer.show();

    std::string img = "../examples/TexShader/out_800*800.png";
    frame.flip();
    frame.draw(img.c_str());

    delete marry;
}

int main()
{
    Lab_0();
    return 0;
}