#include "SSAO.h"

namespace QGL {

float _get_max_slope(Zbuffer &zb, Vec2f pos, Vec2f dir) {
    int width = zb.width;
    int height = zb.height;
    float max_slope = 0.0f;
    for (float t = 0; t < 1000.f; t += 1.0f) {
        Vec2f p = pos + dir*t;
        if (p.x >= width || p.y >= height || p.x < 0 || p.y < 0) return max_slope;
        float dis = t;
        if (dis < 1.f) continue;
        float pz = zb.get(int(p.x), int(p.y));
        // 应该是有一个数组存储是否为背景
        if (pz == -std::numeric_limits<float>::max()) continue;
        // float dh = zb.get(int(p.x), int(p.y)) - zb.get(int(pos.x), int(pos.y));
        // dep value 是负数
        float dh = zb.get(int(pos.x), int(pos.y)) - pz;
        // std::cout << dh << " " << atanf(dh/dis) << std::endl;
        max_slope = std::max(max_slope, atanf(dh/dis));
    }
    // std::cout << max_slope << std::endl;
    return max_slope;    
}

void ScreenSpaceAmbientOcclusion(Frame &frame, Zbuffer &zb) {

    Log log = Log(true, "SSAO: ");

    int width = frame.width;
    int height = frame.height;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (zb.get(i, j) == -std::numeric_limits<float>::max()) continue;
            float total_slope = 0.0f;
            // case 1e-4
            for (float dir = 0; dir < 2*M_PI-1e-4; dir += M_PI/4) {
                total_slope += M_PI/2 - _get_max_slope(zb, Vec2f(i, j), Vec2f(sin(dir), cos(dir)));
            }

            total_slope /= (M_PI/2)*8;

            // std::cout << i << " " << j << " " << zb.get(i, j) << " " << total_slope << std::endl;

            total_slope = std::pow(total_slope, 50.f);  // image will be hard to analysis if not do this.
            
            Vec4f color = Vec4f(total_slope);
            color.w = 1.0f;
            frame.set(i, j, color);

            log.show(i*height+j+1, width*height);
        }
    }
    log.show(width*height, width*height);
}
}