#include "shader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../Utils/stb_image.h"

namespace QGL {
void Shader::texture0(std::string path) {
    if (TEXTURE0.use_count()) TEXTURE0.reset();
    std::cout << "TEXTURE0: ";
    int x, y, n;
    unsigned char *raw = stbi_load(path.c_str(), &x, &y, &n, 0);
    std::cout << "(" << x << ", " << y << ", " << n << ")" << std::endl;
    TEXTURE0 = std::make_shared<Sample2D>(raw, x, y, n);
    delete raw;
}

void Shader::texture0(Vec4f color) {
    if (TEXTURE0.use_count()) TEXTURE0.reset();
    std::cout << "TEXTURE0: " << color << std::endl;;
    TEXTURE0 = std::make_shared<Sample2D>(color);
}
}