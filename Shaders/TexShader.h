#ifndef __TEXSHADERS_H__
#define __TEXSHADERS_H__

#include "shader.h"

namespace QGL {

class TexShader : public Shader {
public:
    StrangeModel *model;

    Matrix uniform_mat_transform;
    Vec2f varying_uv[3];
    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f vertex = Vec4f(in.v, 1.0f);
        float depth = vertex[2];
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        vertex[2] = depth;
        out.sCoord = vertex;

        // tex
        Vec2f uv = model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFrag &in, OutFrag &out) {
        Vec3f bar = in.bar;
        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        model->sampleDiffuse(uv, color);
        out.color = color;
        return false;
    }
};

}

#endif