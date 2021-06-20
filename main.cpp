#include <iostream>

#include "Rasterizer/rasterizer.h"

using namespace QGL;

void Test00() {
    const int width = 800;
    const int height = 800;
    const int depth = 1;
    const float near = 1.f;
    const float far = 150.f;

    const Vec3f camera(0,0,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);

    LookAt(camera, origin, up);
    // SetPerspectiveProjectMat(near, far);
    // SetOrthogonalProjectMat(width, height, depth);
    // SetScreenMat(0, 0, width, height, depth);

    // Init();

    // 加载模型
}

int main()
{
    Test00();
    return 0;
}