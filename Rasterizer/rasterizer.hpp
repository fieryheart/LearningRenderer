#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__
#include <vector>
#include "../Math/vec.hpp"
#include "../QGL/pieces.hpp"
#include "../QGL/transform.hpp"
#include "../QGL/types.h"
#include "../Shaders/Shaders.hpp"

namespace QGL {

// 线程数量
extern int NUM_THREADS;

class Rasterizer {
public:
    Rasterizer() {
        shadingType = ST_Forwad;
        comType = CT_Single;
        model = nullptr;
        shader = nullptr;
    }
    ~Rasterizer() {
        model = nullptr;
        shader = nullptr;
        lights.clear();
        lights.shrink_to_fit();
    }

    void render();
    void draw(std::string path, bool flip=true);
    void shadow(Matrix model_mat);

    Object* model;
    std::vector<Light*> lights;
    Frame frame;
    Zbuffer zbuffer;
    Shader* shader;
    ShadingType shadingType;
    ComType comType;
    Log log;
private:
    void single();
    void forward();
    void scan();

    Vec4f screen_vertexes[3];
};
}

#endif // __RASTERIZER_H__