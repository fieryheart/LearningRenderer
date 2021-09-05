#ifndef __TEXSHADERS_H__
#define __TEXSHADERS_H__

#include "shader.hpp"

namespace QGL {
class TexShader : public Shader {
    Vec2f varying_uv[3];
    Vec3f varying_normal[3];
public:
    Matrix uniform_mat_world2view;
    Matrix uniform_mat_transform;
    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f vertex = Vec4f(in.vertex, 1.0f);
        float depth = (uniform_mat_world2view*vertex).z;
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        vertex[2] = depth;
        out.vertex = vertex;

        // 纹理
        Vec2f uv = in.model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;

        // 法向量
        varying_normal[in.nthvert] = in.model->norm(in.nthface, in.nthvert);
    }

    virtual bool fragment(const InFrag &in, OutFrag &out) {
        Vec3f bar = in.bar;
        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color(1.0, 1.0, 1.0, 1.0);
        TEXTURE0->sample(uv, color);

        Vec3f N = (varying_normal[0]*bar[0]+varying_normal[1]*bar[1]+varying_normal[2]*bar[2]).normalize();
        float k = 0.0f;
        for (auto &light : in.lights) {
            Vec3f wi = -light->Dir();
            float theta = dot(wi, N);
            if (theta > 0) k += dot(wi, N);
        }

        out.color = color * k;
        out.color[3] = 1.0;
        return false;
    }
};

class DirectLightDepthShader : public Shader {
public:
    Matrix uniform_mat_transform;
    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f vertex = Vec4f(in.vertex, 1.0f);
        // std::cout << vertex << std::endl;
        vertex = uniform_mat_transform*vertex;
        // std::cout << vertex << std::endl;
        vertex = vertex / vertex.w;
        
        out.vertex = vertex;
    }
    virtual bool fragment(const InFrag &in, OutFrag &out) {
        // std::cout << in.depth << std::endl;
        out.color = Vec4f(Vec3f(1-in.depth), 1.0);
        return false;
    }
};
}

#endif