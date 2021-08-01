#ifndef __SHADOWSHADER_H_
#define __SHADOWSHADER_H_

#include "shader.h"

namespace QGL {
class DepthShader : public Shader {
public:
    StrangeModel *model;

    Matrix uniform_mat_obj2view;
    Matrix uniform_mat_transform;
    int uniform_max_depth;

    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f vertex = Vec4f(in.v, 1.0f);
        float depth = (uniform_mat_obj2view*vertex).z;
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        vertex[2] = depth;
        out.sCoord = vertex;
    }

    virtual bool fragment(const InFrag &in, OutFrag &out) {
        float depth = in.depth;
        // std::cout << (-depth/uniform_max_depth) << std::endl;
        out.color = Vec4f(Vec3f(1.0f, 1.0f, 1.0f)*(-depth/uniform_max_depth), 1.0f);
        return false;
    }
};

class ShadowShader : public Shader {
public:

    StrangeModel *model;
    Frame *depthmap;

    Matrix uniform_mat_obj2view;
    Matrix uniform_mat_transform;
    Matrix uniform_mat_transform_i;
    Matrix uniform_mat_depth_obj2view;
    Matrix uniform_mat_depth_transform;
    Matrix uniform_mat_norm_transform;
    Vec3f uniform_camera;
    Vec3f uniform_light;
    int uniform_max_depth;

    Vec4f varying_svertex[3];
    Vec3f varying_normal[3];
    Vec2f varying_uv[3];

    virtual void vertex(const InVert &in, OutVert &out) {
        Vec4f _normal = Vec4f(model->norm(in.nthface, in.nthvert), 0.0f);
        _normal = (uniform_mat_norm_transform*_normal).normalize();
        varying_normal[in.nthvert] = _normal.v3f();

        Vec4f vertex = Vec4f(in.v, 1.0f);
        // varying_vertex[in.nthvert] = in.v;
        float depth = (uniform_mat_obj2view*vertex).z;
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        varying_svertex[in.nthvert] = vertex;
        vertex[2] = depth;
        out.sCoord = vertex;

        // tex
        Vec2f uv = model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFrag &in, OutFrag &out) {
        Vec3f bar = in.bar;
        Vec4f v = varying_svertex[0]*bar[0]+varying_svertex[1]*bar[1]+varying_svertex[2]*bar[2];
        v = uniform_mat_transform_i*v;
        v = v / v.w;

        // 光源坐标系下深度值
        Vec4f light_v = uniform_mat_depth_obj2view*v;
        light_v = light_v / light_v.w;
        float z = light_v.z;

        // 深度图
        Vec4f depth_v = uniform_mat_depth_transform*v;
        depth_v = depth_v / depth_v.w;
        Vec4f depth_c = depthmap->get(int(depth_v.x), int(depth_v.y));
        float d = -depth_c[0] * uniform_max_depth;

        // 漫反射、高光、环境光系数
        float Kd = 0.8;
        float Ks = 0.5;
        float Ka = 0.2;

        // sample
        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        model->sampleDiffuse(uv, color);     
        
        if (d-0.1 > z) {
            // out.color = color*0.5;
            out.color = Vec4f(0,0,0,1);
        } else {
            Vec3f dir = (uniform_light-v.v3f()).normalize();

            Vec3f N = (varying_normal[0]*bar[0]+varying_normal[1]*bar[1]+varying_normal[2]*bar[2]).normalize();
            float diffuse = std::max(0.0f, dot(dir, N));

            Vec3f view = uniform_camera-v.v3f();
            Vec3f h = (view + dir).normalize();
            float a = std::max(0.0f, dot(h, N));
            float specular = std::pow(a, 5.f);

            color = color*(Kd*diffuse+Ks*specular+Ka);
            color[3] = 1.0f;
            out.color = color;
        }
        return false;
    }
};
}

#endif // __SHADOWSHADER_H_