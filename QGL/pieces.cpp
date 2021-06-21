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

Frame Frame::copy() {
    Frame ret = Frame(width, height, 4);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            ret.set(i, j, buffer[i+width*j]);
        }
    }
    return ret;
}

void Frame::filter(std::vector<float> &kernel, int kw, int kh) {
    Frame frame_copy = copy();

    for (int i = kw/2; i < width-kw/2; ++i) {
        for (int j = kh/2; j < height-kh/2; ++j) {
            Vec4f c = Vec4f(0.0f), c1;
            for (int k = 0; k < kernel.size(); ++k) {
                int ii = k % kw - 1 + i;
                int jj = k / kh - 1 + j;
                c = c + (frame_copy.get(ii, jj)) * kernel[k];
            }
            set(i, j, c);
        }
    }    
}

}