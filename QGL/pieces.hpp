#ifndef __PIECES_H__
#define __PIECES_H__

#include <chrono>
#include <iostream>
#include <random>
#include "../Math/math_init.hpp"

using namespace std::chrono;

namespace QGL {

//>>>=========================================
//
//              自定义的数据结构
//
//>>>=========================================

// 帧结构
struct Frame {
    int width;
    int height;
    int channel;
    int id;
    std::vector<Vec4f> buffer;
    Frame() {}
    ~Frame(){}
    Frame(int w, int h, int n) : width(w), height(h), channel(n) {
        buffer = std::vector<Vec4f>(w*h, Vec4f(Vec3f(0.0f), 1.0f));
    }
    Vec4f get(int i, int j) {
        return buffer[i+width*j];
    }
    void set(int i, int j, Vec4f color) {
        buffer[i+width*j] = color;
    }
    void draw(const char *filename);
    void flip() {
        for (int i = 0; i < height / 2; ++i) {
            for (int j = 0; j < width; ++j) {
                Vec4f t = buffer[j+width*i];
                buffer[j+width*i] = buffer[j+width*(height-1-i)];
                buffer[j+width*(height-1-i)] = t;
            }
        }
    }
    Frame copy() {
        Frame ret = Frame(width, height, 4);
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                ret.set(i, j, buffer[i+width*j]);
            }
        }
        return ret;
    }
    void clear() {
        buffer = std::vector<Vec4f>(width*height, Vec4f(Vec3f(0.0f), 1.0f));
    }
    void filter(std::vector<float> &kernel, int kw, int kh) {
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
};

// Zbuffer结构
struct Zbuffer {
    int width;
    int height;
    std::vector<float> zbuffer;
    Zbuffer() {}
    ~Zbuffer() {}
    Zbuffer(int w, int h) : width(w), height(h){
        zbuffer = std::vector<float>(w*h, 1.0);
    }
    float get(int i, int j) {
        return zbuffer[i+width*j];
    }
    void set(int i, int j, float z) {
        zbuffer[i+width*j] = z;
    }
    void clear() {
        std::fill(zbuffer.begin(), zbuffer.end(), 1.0);
    }
};

// Sample2D结构
struct Sample2D {
    int width;
    int height;
    int channel;
    std::vector<float> data;
    Sample2D(unsigned char *raw, int w, int h, int n) : width(w), height(h), channel(n) {
        for (int i = 0; i < w*h*n; ++i) {
            float val = ((int)raw[i])/255.0f;
            data.push_back(val);
        }
    }
    Sample2D(Vec4f color){
        width = 1, height = 1, channel = 4;
        for (int i = 0; i < 4; ++i) {
            float val = ((int)color[i])/255.0f;
            data.push_back(val);
        }
    }
    void sample(Vec2f uv, Vec4f &val) {
        int x = int(uv[0]*width), y = int(uv[1]*height);
        x = std::max(0, std::min(width-1, x));
        y = std::max(0, std::min(height-1, y));
        // std::cout << "(u, v): " << uv[0] << " " << uv[1] << " ";
        // std::cout << "(x, y): " << x << " " << y << std::endl;
        int index = (y*width+x)*channel;
        if (channel == 1) {
            val[0] = data[index];
            val[1] = data[index];
            val[2] = data[index];
            val[3] = 1.0f;
        } else if (channel == 3) {
            val[0] = data[index];
            val[1] = data[index+1];
            val[2] = data[index+2];
            val[3] = 1.0f;
        } else if (channel == 4) {
            val[0] = data[index];
            val[1] = data[index+1];
            val[2] = data[index+2];
            val[3] = data[index+3];
        }
    }
};

//>>>=========================================
//
//                辅助工具类
//
//>>>=========================================

// 随机发生器
template <class T1, class T2> struct Randomizer {
    std::default_random_engine random;
    T1 dist;
    Randomizer<T1, T2>() {}
    Randomizer<T1, T2>(std::default_random_engine r, T1 d) : random(r), dist(d) {}
    T2 get() {return dist(random);}
};


// 计时器
class Timer {
public:
    Timer() {update();}
    ~Timer() {}
    void update() {_start = high_resolution_clock::now();}
    double second() {return microsec()*0.000001;}
    double millisec() {return microsec()*0.001;}
    long long microsec() {return duration_cast<microseconds>(high_resolution_clock::now() - _start).count();}
    void show() {
        double t = millisec();
        if (t < 1000) {
            std::cout << t << "ms" << std::endl;
        } else if (t >= 1000 && t < 1000*3600) {
            std::cout << t/1000 << "s" << std::endl;
        } else {
            std::cout << t/(1000*3600) << "h" << std::endl;
        }
        
    }
private:
    time_point<high_resolution_clock>_start;
};

// Log
struct Log {
    bool flag;
    std::string prefix;
    Log() {}
    ~Log() {}
    Log(bool flag, std::string prefix) : flag(flag), prefix(prefix){}
    void show(int i, int f){
        if (i < f) {
            std::cerr << "\r" << prefix;
            // printf("%.2f", i*1.0/f*100);
            fprintf(stderr, "%.2f", i*1.0/f*100);
            std::cerr << "% completed..." << std::flush;
            // std::cerr << "\r" << prefix << i*1.0/f*100 << "% completed." << std::flush;
        } else {
            std::cerr << "\r";
            for(int i = 0;i < 100; ++i) std::cerr << " ";
            std::cerr.flush();
            std::cerr << "\r" << prefix << "100% completed." << std::endl;
        }
    }
};

}

#endif  // __PIECES_H__