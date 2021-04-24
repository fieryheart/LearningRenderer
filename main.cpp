#include <vector>
#include <iostream>
#include <algorithm>
#include "qgl.h"

Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth = 255;  // 根据深度值 计算颜色
const Vec3f light1_dir(1,1,1);
const Vec3f camera(1,1,10);
const Vec3f origin(0,0,0);
const Vec3f up(0,1,0);
Matrix ModelMatrix;       // 模型空间
Matrix ViewMatrix;        // 视角空间
Matrix ProjectMatrix;     // 投影空间
Matrix ViewportMatrix;    // 屏幕空间
Matrix TransformMatrix;

class GouraudShader : public Shader {
    Vec3f varying_intensity;
public:
    virtual Vec4f vertex(int nthface, int nthvert) {
        varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light1_dir, 0.f, 1.f);
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = ViewportMatrix*ProjectMatrix*ViewMatrix*ModelMatrix*gl_vertex;
        // std::cout << gl_vertex << std::endl;s
        return  gl_vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};

class ToonShader : public Shader {
    Vec3f varying_intensity;
public:
    virtual Vec4f vertex(int nthface, int nthvert) {
        varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light1_dir, 0.f, 1.f);
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = TransformMatrix*gl_vertex;
        // std::cout << gl_vertex << std::endl;s
        return  gl_vertex/gl_vertex[3];
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        if (intensity>.85) intensity = 1;
        else if (intensity>.60) intensity = .80;
        else if (intensity>.45) intensity = .60;
        else if (intensity>.30) intensity = .45;
        else if (intensity>.15) intensity = .30;
        else intensity = 0;
        color = TGAColor(255, 155, 0)*intensity;
        return false;
    }
};

class PhongShader : public Shader {
public:
    // 变换矩阵
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Matrix mat_normal, mat_normal_it;

    Vec4f gl_vertex[3];
    Vec3f tex_uv[3];

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;
        mat_normal = mat_project*mat_view*mat_model;
        mat_normal_it = mat_normal.inverse().transpose();
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        // varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light1_dir, 0.f, 1.f);
        gl_vertex[nthvert] = model->vert(nthface, nthvert);
        gl_vertex[nthvert] = mat_transform*gl_vertex[nthvert];
        tex_uv[nthvert] = model->texCoord(nthface, nthvert);
        return  gl_vertex[nthvert]/gl_vertex[nthvert][3];
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        // float intensity = varying_intensity*bar;
        Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
        Vec4f n(model->normal(uv),0.f);
        n = (mat_normal_it*n).normalize();
        Vec4f l(light1_dir, 0.f);
        l = (mat_normal*l).normalize();

        Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

        float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);

        TGAColor c = model->diffuse(uv);

        // std::cout << n << " " << l << std::endl;
        // std::cout << (int)c[0] << " " << (int)c[1] << " " << (int)c[2] << std::endl;
        
        for (int i=0; i<3; i++) {
            // 5. : 环境光部分
            // 1. : 漫反射部分
            // .6 : 高光部分
            color[i] = std::min<float>(5. + c[i]*(1.*diff + .6*spec), 255);
        }

        return false;
    }
};

class DepthShader : public Shader {
public:
    // 变换矩阵
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform, mat_transform_it;

    // 顶点相关数据
    float gl_depth[3];

    void computeTranform() {
        mat_transform = mat_viewport*mat_view*mat_model;
    }

    virtual Vec4f vertex(int nthface, int nthvert) { 
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = mat_transform*gl_vertex;
        gl_vertex = gl_vertex/gl_vertex[3];
        gl_depth[nthvert] = gl_vertex[2];
        return gl_vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        int d = (int)(bar[0]*gl_depth[0]+bar[1]*gl_depth[1]+bar[2]*gl_depth[2]);
        color = TGAColor(d, d, d);
        // std::cout << d << std::endl;
        return false;
    }
};

class ShadowShader : public Shader {
public:
    Matrix mat_transform_depth;
    PhongShader phongShader;
    Vec4f gl_vertex[3];

    virtual Vec4f vertex(int nthface, int nthvert) { 
        gl_vertex[nthvert] = model->vert(nthface, nthvert);
        return phongShader.vertex(nthface, nthvert);    
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        phongShader.fragment(bar, color);
        Vec4f v = gl_vertex[0]*bar[0]+gl_vertex[1]*bar[1]+gl_vertex[2]*bar[2];
        v = mat_transform_depth*v;
        v = v / v[3];
        int d = model->depthmap.get(int(v[0]+.5f), int(v[1]+.5f))[0];
        // float shadow = .3f;
        if (v[2] < d-1) {
            // 产生阴影
            color = TGAColor(10, 10, 10);
        }
        return false;
    }
};

void shading(TGAImage &image, TGAImage &zbuffer, Shader &shader) {
    Vec4f screen_coords[3];
    for (int i=0; i<model->nfaces(); i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }
}

void start() {
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    // 初始化各转换矩阵
    ModelMatrix = GetModelMatrix();
    ViewMatrix = GetViewMatrix(camera, origin, up);
    ProjectMatrix = GetProjectMatrix(camera, origin);
    ViewportMatrix = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    TransformMatrix = ViewportMatrix*ProjectMatrix*ViewMatrix*ModelMatrix;

    // Shading
    DepthShader depthShader;
    depthShader.mat_model = GetModelMatrix();
    depthShader.mat_view = GetViewMatrix(light1_dir, origin, up);
    depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    depthShader.computeTranform();
    // 获取深度数据
    model->depthmap = TGAImage(width, height, TGAImage::RGB);
    shading(model->depthmap, zbuffer, depthShader);

    // Phong Shading
    PhongShader phongShader;
    phongShader.mat_model = GetModelMatrix();
    phongShader.mat_view = GetViewMatrix(camera, origin, up);
    phongShader.mat_project = GetProjectMatrix(camera, origin);
    phongShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    phongShader.computeTranform();

    // Shadow Shading
    ShadowShader shadowShader;
    shadowShader.mat_transform_depth = depthShader.mat_transform;
    shadowShader.phongShader = phongShader;
    zbuffer.clear();
    shading(image, zbuffer, shadowShader);


    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    // zbuffer.flip_vertically();
    // zbuffer.write_tga_file("zbuffer.tga");

    model->depthmap.flip_vertically();
    model->depthmap.write_tga_file("output_depth.tga");
}

int main(int argc, char** argv) {

    model = new Model("../obj/african_head.obj");

    start();

    delete model;
    return 0;
}