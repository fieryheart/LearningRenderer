#include "pieces.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Utils/stb_image_write.h"


namespace QGL {

void Frame::draw(const char *filename) {
    auto data = (unsigned char*)malloc(width*height*channel);
    for (int i = 0; i < width*height; ++i) {
        for (int j = 0; j < channel; ++j) {
            data[i*channel+j] = (unsigned char)(255*std::max(0.0f, std::min(1.0f, buffer[i][j])));
        }
    }
    stbi_write_png(filename, width, height, channel, data, 0);
}

void Frame::flip() {
    for (int i = 0; i < height / 2; ++i) {
    for (int j = 0; j < width; ++j) {
            Vec4f t = buffer[j+width*i];
            buffer[j+width*i] = buffer[j+width*(height-1-i)];
            buffer[j+width*(height-1-i)] = t;
        }
    }
}

}