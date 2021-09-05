#ifndef __SHADERS_H__
#define __SHADERS_H__
#include <vector>
#include "../Objects/object.hpp"

namespace QGL {

// Shader中用到的
struct InVert {
    Vec3f vertex;
    int nthface;
    int nthvert;
    Object* model;
    InVert(){}
};

struct OutVert {
    Vec4f vertex;   // 屏幕坐标
    float depth;
    OutVert(){}
};

struct InFrag {
    Vec3f bar;
    float depth;
    Object* model;
    std::vector<Light*> lights;
    InFrag(){}
};

struct OutFrag {
    Vec4f color;
    OutFrag(){}
};


class Shader {
public:
    std::shared_ptr<Sample2D> TEXTURE0, TEXTURE1, TEXTURE2, TEXTURE3;
    Shader() {}
    void texture0(std::string path);
    void texture0(Vec4f color);
    virtual ~Shader() {};
    virtual void vertex(const InVert &in, OutVert &out) {};
    virtual bool fragment(const InFrag &in, OutFrag &out) {};
};

}

#endif 