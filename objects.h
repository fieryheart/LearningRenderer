#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <chrono>
#include <iostream>
#include "geometry.h"

using namespace std::chrono;

namespace QGL {
// 帧结构
struct Frame {
    int width;
    int height;
    int channel;
    int id;
    std::vector<Vec4f> buffer;
    Frame(int w, int h, int n) : width(w), height(h), channel(n) {
        buffer = std::vector<Vec4f>(w*h, Vec4f(Vec3f(0.0f), 1.0f));
    }
    ~Frame(){}
    Vec4f get(int i, int j) {
        return buffer[i+width*j];
    }
    void set(int i, int j, Vec4f color) {
        buffer[i+width*j] = color;
    }
    void draw(const char *filename);
    void flip();
};

// Zbuffer结构
struct Zbuffer {
    int width;
    int height;
    std::vector<float> zbuffer;
    Zbuffer(int w, int h) : width(w), height(h){
        zbuffer = std::vector<float>(w*h, 0.0f);
    }
    float get(int i, int j) {
        return zbuffer[i+width*j];
    }
    void set(int i, int j, float z) {
        zbuffer[i+width*j] = z;
    }
    void clear() {
        std::fill(zbuffer.begin(), zbuffer.end(), 0.0f);
    }
};

// 计算方式
enum ComType {
    CT_Single = 0,
    CT_Omp
};

// 贴图类型
enum MapType {
    MT_Diffuse = 0,
    MT_Normal,
    MT_Specular
};

// 模型类型
enum ModelType {
	MDT_Buildin = 0,
    MDT_Strange,
    MDT_Light
};

enum MatrialType {
    MTT_Diffuse = 0,
    MTT_Mirror,
    MTT_Glossy
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

// Log
struct Log {
    bool flag;
    std::string prefix;
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

// 计时器
class Timer {
public:
    Timer() {update();}
    ~Timer() {}
    void update() {_start = high_resolution_clock::now();}
    double second() {return microsec()*0.000001;}
    double millisec() {return microsec()*0.001;}
    long long microsec() {return duration_cast<microseconds>(high_resolution_clock::now() - _start).count();}
private:
    time_point<high_resolution_clock>_start;
};

// 光源
struct Light {
    virtual ~Light() {};
};

struct DirectLight : Light {
    Vec3f pos;
    Vec3f dir;
    DirectLight(Vec3f pos, Vec3f dir) : pos(pos), dir(dir) {} 
};

struct PointLight : Light {
    Vec3f pos;        // 位置
    float illumination;
    PointLight(Vec3f pos, float illumination) : pos(pos), illumination(illumination) {}
};

struct Ray {
    Vec3f pos;
    Vec3f dir;
    Ray(){}
    Ray(Vec3f pos, Vec3f dir) : pos(pos), dir(dir) {}
    ~Ray(){}
    Vec3f launch(float t) {return pos+dir*t;}
};

// 
struct Object {
    virtual ~Object() {};
    virtual bool interact(Ray &ray, float &t) = 0;
};


// 三角形：点、法向量
// class Triangle : public Object {
// private:
//     Vec3f points[3];
//     Vec3f normal;
// public:
//     Triangle() {};
//     ~Triangle() {};
//     Triangle(Vec3f *_points) {points[0] = _points[0]; points[1] = _points[1]; points[2] = _points[2];}
//     Triangle(Vec3f *_points, Vec3f _normal) {points[0] = _points[0]; points[1] = _points[1]; points[2] = _points[2]; normal = _normal;}
//     Vec3f getPoints(int index) {
//         if (index >= 0 && index <= 2) throw "Index is more than 2.";
//         return points[index];
//     }
//     void setPoints(int index, Vec3f point) {
//         if (index >= 0 && index <= 2) throw "Index is more than 2.";
//         points[index] = point;
//     }
//     Vec3f getNormal() {
//         return normal;
//     }
//     void setNormal(Vec3f _normal) {
//         normal = _normal;
//     }
// };

// struct Triangle : Object {
//     Vec3f points[3];
//     Vec3f normal;
//     Vec3f minp, maxp;
//     Vec3f centroid;
//     Triangle() {}
//     Triangle(Vec3f A, Vec3f B, Vec3f C) {
//         points[0] = A;
//         points[1] = B;
//         points[2] = C;
//         Vec3f ab = B - A;
//         Vec3f ac = C - A;
//         normal = (ac^ab).normalize();
//         for (int i = 0; i < 3; ++i) {
//             minp[i] = std::min(A[i], std::min(B[i], C[i]));
//             maxp[i] = std::max(A[i], std::max(B[i], C[i]));
//             centroid[i] = (minp[i]+maxp[i])/2.0f;
//         }
//     }
// }
}
#endif // __OBJECTS_H__