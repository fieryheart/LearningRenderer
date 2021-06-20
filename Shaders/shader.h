#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "../Models/model.h"

namespace QGL {

// Shader中用到的
struct InVert {
    Vec3f v;
    int nthface;
    int nthvert;
    Model *model;
    InVert(){}
};

struct OutVert {
    Vec4f sCoord;   // 屏幕坐标
    OutVert(){}
};

struct InFrag {
    Vec3f bar;
    float depth;
    Model *model;
    InFrag(){}
};

struct OutFrag {
    Vec4f color;
    OutFrag(){}
};


class Shader {
public:
    virtual ~Shader() {};
    virtual void vertex(const InVert &in, OutVert &out) = 0;
    virtual bool fragment(const InFrag &in, OutFrag &out) = 0;
};

}

#endif 