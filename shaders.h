#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "objects.h"

namespace QGL {
struct InVectex {
    Vec3f v;
    int nthface;
    int nthvert;
    Model *model;
    InVectex(){}
};

struct OutVectex {
    Vec4f sCoord;   // 屏幕坐标
    OutVectex(){}
};

struct InFragment {
    Vec3f bar;
    float depth;
    Model *model;
    InFragment(){}
};

struct OutFragment {
    Vec4f color;
    OutFragment(){}
};

class Shader {
public:
    virtual ~Shader() {};
    virtual void vertex(const InVectex &in, OutVectex &out) = 0;
    virtual bool fragment(const InFragment &in, OutFragment &out) = 0;
};

class TestShader : public Shader {
public:
    Matrix uniform_mat_transform;
    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        out.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
        return false;
    }
};

// 根据深度值着色
class DepthShader : public Shader {
public:
    Matrix uniform_mat_transform;
    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        // float depth = 1.0f-std::pow(cos(in.depth*M_PI/2), 1);
        float depth = in.depth;
        out.color = Vec4f(Vec3f(1.0f, 1.0f, 1.0f)*depth, 1.0f);
        return false;
    }    
};

// 纹理
class TexShader : public Shader {
public:
    Matrix uniform_mat_transform;
    Vec2f varying_uv[3];
    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;

        // tex
        Vec2f uv = in.model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        Vec3f bar = in.bar;
        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        in.model->sampleDiffuse(uv, color);
        out.color = color;
        return false;
    }
};

// 
class GouraudShader : public Shader {
public:
    Matrix uniform_mat_transform;
    Matrix uniform_mat_norm_transform;
    Vec3f uniform_light;
    Vec3f varying_intensity;
    Vec2f varying_uv[3];

    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f _normal = Vec4f(in.model->norm(in.nthface, in.nthvert), 0.0f);
        _normal = (uniform_mat_norm_transform*_normal).normalize();
        varying_intensity[in.nthvert] = std::clamp(_normal.v3f()*uniform_light, 0.f, 1.f);

        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;

        // tex
        Vec2f uv = in.model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        Vec3f bar = in.bar;
        float intensity = varying_intensity*bar;
        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        in.model->sampleDiffuse(uv, color);
        color = color*intensity;
        color[3] = 1.0f;
        out.color = color;
        return false;
    }    
};

// no shadow
class PhongShader : public Shader {
public:
    Matrix uniform_mat_transform;
    Matrix uniform_mat_norm_transform;
    Vec3f uniform_camera;
    Vec3f uniform_light;
    Vec3f varying_vertex[3];
    Vec3f varying_normal[3];
    Vec2f varying_uv[3];
    

    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f _normal = Vec4f(in.model->norm(in.nthface, in.nthvert), 0.0f);
        _normal = (uniform_mat_norm_transform*_normal).normalize();
        varying_normal[in.nthvert] = _normal.v3f();

        varying_vertex[in.nthvert] = in.v;
        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;

        // tex
        Vec2f uv = in.model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        Vec3f bar = in.bar;
        Vec3f dir = uniform_light;

        Vec3f N = varying_normal[0]*bar[0]+varying_normal[1]*bar[1]+varying_normal[2]*bar[2];
        float diffuse = std::max(0.0f, dir*N);

        Vec3f v = varying_vertex[0]*bar[0]+varying_vertex[1]*bar[1]+varying_vertex[2]*bar[2];
        Vec3f view = uniform_camera-v;
        Vec3f h = (view + dir).normalize();
        float a = std::max(0.0f, h*N);
        float specular = std::pow(a, 100.f);

        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        in.model->sampleDiffuse(uv, color);

        float Kd = 0.8;
        float Ks = 0.0;
        float Ka = 1.0;

        color = color*(Kd*diffuse+Ks*specular)*1.3f+Ka/255.0f;
        color[3] = 1.0f;
        out.color = color;
        return false;
    }    
};

class ShadowShader : public Shader {
public:
    Matrix uniform_mat_transform;
    Matrix uniform_mat_norm_transform;
    Matrix uniform_mat_depth_transform;
    Vec3f uniform_camera;
    Vec3f uniform_light;
    Vec3f varying_vertex[3];
    Vec3f varying_normal[3];
    Vec2f varying_uv[3];
    Frame *depthFrame;

    virtual void vertex(const InVectex &in, OutVectex &out) {
        Vec4f _normal = Vec4f(in.model->norm(in.nthface, in.nthvert), 0.0f);
        _normal = (uniform_mat_norm_transform*_normal).normalize();
        varying_normal[in.nthvert] = _normal.v3f();

        varying_vertex[in.nthvert] = in.v;
        Vec4f vertex = Vec4f(in.v, 1.0f);
        vertex = uniform_mat_transform*vertex;
        vertex = vertex / vertex.w;
        out.sCoord = vertex;

        // tex
        Vec2f uv = in.model->tex(in.nthface, in.nthvert);
        varying_uv[in.nthvert] = uv;
    }

    virtual bool fragment(const InFragment &in, OutFragment &out) {
        Vec3f bar = in.bar;
        Vec3f dir = uniform_light;
        Vec3f v = varying_vertex[0]*bar[0]+varying_vertex[1]*bar[1]+varying_vertex[2]*bar[2];
        
        // if shadow
        Vec4f v4f = Vec4f(v, 1.0f);
        v4f = uniform_mat_depth_transform*v4f;
        v4f = v4f / v4f.w;
        float z = depthFrame->get(int(v4f.x+.5f), int(v4f.y+.5f))[0];
        if (v4f.z > z + 0.003f) {
            out.color = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
            return false;
        }

        Vec3f N = varying_normal[0]*bar[0]+varying_normal[1]*bar[1]+varying_normal[2]*bar[2];
        float diffuse = std::max(0.0f, dir*N);

        Vec3f view = uniform_camera-v;
        Vec3f h = (view + dir).normalize();
        float a = std::max(0.0f, h*N);
        float specular = std::pow(a, 100.f);

        Vec2f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f color;
        in.model->sampleDiffuse(uv, color);

        float Kd = 0.8;
        float Ks = 0.0;
        float Ka = 1.0;

        color = color*(Kd*diffuse+Ks*specular)*1.3f+Ka/255.0f;
        color[3] = 1.0f;
        out.color = color;
        return false;
    }    
};
}


// class GouraudShader : public Shader {
// public:
//     Model *model;
//     Vec3f varying_intensity;
//     Matrix mat_model, mat_view, mat_project, mat_viewport;
//     Matrix mat_transform;
//     Vec3f light;

//     void computeTranform() {
//         mat_transform = mat_viewport*mat_project*mat_view*mat_model;    
//     }

//     virtual Vec4f vertex(int nthface, int nthvert) {
//         varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light, 0.f, 1.f);
//         Vec4f gl_vertex = model->vert(nthface, nthvert);
//         gl_vertex = mat_transform*gl_vertex;
//         // std::cout << gl_vertex << std::endl;s
//         return  gl_vertex;
//     }
//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         float intensity = varying_intensity*bar;
//         color = TGAColor(255, 255, 255)*intensity;
//         return false;
//     }
// };

// // Phong Shader with no shadow
// class PhongShader : public Shader {
// public:
//     Model *model;
//     // 变换矩阵
//     Matrix mat_model, mat_view, mat_project, mat_viewport;
//     Matrix mat_transform;
//     Matrix mat_normal, mat_normal_it;
//     Vec3f light;

//     Vec4f gl_vertex[3];
//     Vec3f tex_uv[3];

//     void computeTranform() {
//         mat_transform = mat_viewport*mat_project*mat_view*mat_model;
//         mat_normal = mat_project*mat_view*mat_model;
//         mat_normal_it = mat_normal.inverse().transpose();
//     }

//     virtual Vec4f vertex(int nthface, int nthvert) {
//         gl_vertex[nthvert] = model->vert(nthface, nthvert);
//         gl_vertex[nthvert] = mat_transform*gl_vertex[nthvert];
//         tex_uv[nthvert] = model->texCoord(nthface, nthvert);
//         return  gl_vertex[nthvert]/gl_vertex[nthvert][3];
//     }
//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
//         Vec4f n(model->normal(uv),0.f);
//         n = (mat_normal_it*n).normalize();
//         Vec4f l(light, 0.f);
//         l = (mat_normal*l).normalize();

//         Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

//         float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
//         float diff = std::max(0.f, n*l);

//         TGAColor c = model->diffuse(uv);
        
//         for (int i=0; i<3; i++) {
//             // 5. : 环境光部分
//             // 1. : 漫反射部分
//             // .6 : 高光部分
//             color[i] = std::min<float>(5. + c[i]*(1.*diff + .6*spec), 255);
//         }

//         return false;
//     }
// };

// // Depth Shader with custom camera
// class DepthShader : public Shader {
// public:
//     Model *model;
//     // 变换矩阵
//     Matrix mat_model, mat_view, mat_project, mat_viewport;
//     Matrix mat_transform, mat_transform_it;

//     // 顶点相关数据
//     float gl_depth[3];

//     void computeTranform() {
//         mat_transform = mat_viewport*mat_project*mat_view*mat_model;
//     }

//     virtual Vec4f vertex(int nthface, int nthvert) { 
//         Vec4f gl_vertex = model->vert(nthface, nthvert);
//         gl_vertex = mat_transform*gl_vertex;
//         gl_vertex = gl_vertex/gl_vertex[3];
//         gl_depth[nthvert] = gl_vertex[2];
//         return gl_vertex;
//     }

//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         int d = (int)(bar[0]*gl_depth[0]+bar[1]*gl_depth[1]+bar[2]*gl_depth[2]);
//         color = TGAColor(d, d, d)*255.0f;
//         return false;
//     }
// };

// // Phong Shader with shadow
// class ShadowShader : public Shader {
// public:
//     Model *model;
//     Matrix mat_model, mat_view, mat_project, mat_viewport;
//     Matrix mat_transform;
//     Matrix mat_normal, mat_normal_it;
//     Matrix mat_transform_depth;
//     Vec3f light;
//     TGAImage *depthmap;

//     Vec4f vertexes[3];
//     Vec4f gl_vertexes[3];
//     Vec3f tex_uv[3];

//     void computeTranform() {
//         mat_transform = mat_viewport*mat_project*mat_view*mat_model;
//         mat_normal = mat_project*mat_view*mat_model;
//         mat_normal_it = mat_normal.inverse().transpose();
//     }

//     virtual Vec4f vertex(int nthface, int nthvert) {
//         vertexes[nthvert] = model->vert(nthface, nthvert);
//         gl_vertexes[nthvert] = mat_transform*vertexes[nthvert];
//         tex_uv[nthvert] = model->texCoord(nthface, nthvert);
//         return  gl_vertexes[nthvert]/gl_vertexes[nthvert][3];
//     }

//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         // 纹理坐标
//         Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
//         Vec4f n(model->normal(uv),0.f);
//         n = (mat_normal_it*n).normalize();
//         Vec4f l(light, 0.f);
//         l = (mat_normal*l).normalize();

//         Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

//         float spec = (std::max(r.z, 0.0f), model->specular(uv));
//         float diff = std::max(0.f, n*l);

//         // 漫反射
//         TGAColor c = model->diffuse(uv);

//         // 阴影
//         Vec4f depth_vectex = vertexes[0]*bar[0]+vertexes[1]*bar[1]+vertexes[2]*bar[2];
//         depth_vectex = mat_transform_depth*depth_vectex;
//         depth_vectex = depth_vectex / depth_vectex[3];
//         int d = depthmap->get(int(depth_vectex[0]+.5f), int(depth_vectex[1]+.5f))[0];
//         float shadow = 1.f;
//         int bias = 1;
//         // std::cout << d << std::endl;
//         if (depth_vectex[2] < d-bias) {
//             // std::cout << shadow << std::endl;
//             shadow = .3f;
//         }
        
//         // Phong + Shadow
//         for (int i=0; i<3; i++) {
//             // 5. : 环境光部分
//             // 1. : 漫反射部分
//             // .6 : 高光部分
//             color[i] = std::min<float>(5. + c[i]*shadow*(1.*diff + .6*spec), 255);
//         }

//         return false;
//     }
// };

// // AO Shader
// class OcclusionShader : public Shader {
// public:
//     Model *model;
//     Matrix mat_model, mat_view, mat_viewport;
//     Matrix mat_transform;
//     Matrix mat_transform_depth;

//     TGAImage *depthmap;
//     TGAImage *occlusionmap;

//     Vec4f vertexes[3];
//     Vec2f tex_uv[3];

//     void computeTranform() {
//         mat_transform = mat_viewport*mat_view*mat_model;
//         // std::cout << mat_transform << std::endl;
//     }

//     virtual Vec4f vertex(int nthface, int nthvert) { 
//         vertexes[nthvert] = model->vert(nthface, nthvert);
//         vertexes[nthvert] = mat_transform*vertexes[nthvert];
//         Vec3f tex = model->texCoord(nthface, nthvert);
//         tex_uv[nthvert].x = tex.x * occlusionmap->get_width();
//         tex_uv[nthvert].y = tex.y * occlusionmap->get_height();
//         return vertexes[nthvert]/vertexes[nthvert][3];
//     }

//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         // 纹理坐标
//         Vec2f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
        
//         // gl坐标
//         Vec4f vertex = vertexes[0]*bar[0]+vertexes[1]*bar[1]+vertexes[2]*bar[2];

//         // std::cout << "depthmap.z: " << (float)((depthmap->get((int)(vertex.x+.5f), (int)(vertex.y+.5f)))[0]) << std::endl; 
//         // std::cout << "vertex.z: " << vertex.z << std::endl;

//         if (std::abs((depthmap->get((int)(vertex.x+.5f), (int)(vertex.y+.5f)))[0]-vertex.z) < 1) {
//             occlusionmap->set((int)(uv.x+.5f), (int)(uv.y+.5f), 255);
//         }
//         color = TGAColor(255, 0, 0);
//         return false;

//     }
// };

#endif // __SHADERS_H__