#ifndef __PHONGSHADER_H_
#define __PHONGSHADER_H_

#include "shader.h"

namespace QGL {
class PhongShader : public Shader {
public:
    StrangeModel *model;

    Matrix uniform_mat_obj2view;
    Matrix uniform_mat_transform;
    Matrix uniform_mat_norm_transform;
    Vec3f uniform_camera;
    Vec3f uniform_light;

    Vec3f varying_vertex[3];
    Vec3f varying_normal[3];
    Vec2f varying_uv[3];
    

    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f _normal = Vec4f(model->norm(in.nthface, in.nthvert), 0.0f);
        _normal = (uniform_mat_norm_transform*_normal).normalize();
        varying_normal[in.nthvert] = _normal.v3f();

        varying_vertex[in.nthvert] = in.v;
        Vec4f vertex = Vec4f(in.v, 1.0f);
        float depth = (uniform_mat_obj2view*vertex).z;
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
        Vec3f dir = -uniform_light;

        Vec3f N = varying_normal[0]*bar[0]+varying_normal[1]*bar[1]+varying_normal[2]*bar[2];
        float diffuse = std::max(0.0f, dir*N);

        Vec3f v = varying_vertex[0]*bar[0]+varying_vertex[1]*bar[1]+varying_vertex[2]*bar[2];
        Vec3f view = uniform_camera-v;
        Vec3f h = (view + dir).normalize();
        float a = std::max(0.0f, h*N);
        float specular = std::pow(a, 5.f);

        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        model->sampleDiffuse(uv, color);

        float Kd = 0.8;
        float Ks = 0.5;
        float Ka = 0.2;

        color = color*(Kd*diffuse+Ks*specular+Ka);
        color[3] = 1.0f;
        out.color = color;
        return false;
    }
};
}

#endif // __PHONGSHADER_H_