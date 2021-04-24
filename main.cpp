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

class GouraudShader : public Shader {
public:
    Vec3f varying_intensity;
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Vec3f light;

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;    
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light, 0.f, 1.f);
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = mat_transform*gl_vertex;
        // std::cout << gl_vertex << std::endl;s
        return  gl_vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};


// Phong Shading with no shadow
class PhongShader : public Shader {
public:
    // 变换矩阵
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Matrix mat_normal, mat_normal_it;
    Vec3f light;

    Vec4f gl_vertex[3];
    Vec3f tex_uv[3];

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;
        mat_normal = mat_project*mat_view*mat_model;
        mat_normal_it = mat_normal.inverse().transpose();
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        gl_vertex[nthvert] = model->vert(nthface, nthvert);
        gl_vertex[nthvert] = mat_transform*gl_vertex[nthvert];
        tex_uv[nthvert] = model->texCoord(nthface, nthvert);
        return  gl_vertex[nthvert]/gl_vertex[nthvert][3];
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
        Vec4f n(model->normal(uv),0.f);
        n = (mat_normal_it*n).normalize();
        Vec4f l(light, 0.f);
        l = (mat_normal*l).normalize();

        Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

        float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);

        TGAColor c = model->diffuse(uv);
        
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
        return false;
    }
};

class ShadowShader : public Shader {
public:
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Matrix mat_normal, mat_normal_it;
    Matrix mat_transform_depth;
    Vec3f light;
    TGAImage *depthmap;

    Vec4f vertexes[3];
    Vec4f gl_vertexes[3];
    Vec3f tex_uv[3];

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;
        mat_normal = mat_project*mat_view*mat_model;
        mat_normal_it = mat_normal.inverse().transpose();
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        vertexes[nthvert] = model->vert(nthface, nthvert);
        gl_vertexes[nthvert] = mat_transform*vertexes[nthvert];
        tex_uv[nthvert] = model->texCoord(nthface, nthvert);
        return  gl_vertexes[nthvert]/gl_vertexes[nthvert][3];
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        // 纹理坐标
        Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
        Vec4f n(model->normal(uv),0.f);
        n = (mat_normal_it*n).normalize();
        Vec4f l(light, 0.f);
        l = (mat_normal*l).normalize();

        Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

        float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);

        // 漫反射
        TGAColor c = model->diffuse(uv);

        // 阴影
        Vec4f depth_vectex = vertexes[0]*bar[0]+vertexes[1]*bar[1]+vertexes[2]*bar[2];
        depth_vectex = mat_transform_depth*depth_vectex;
        depth_vectex = depth_vectex / depth_vectex[3];
        int d = depthmap->get(int(depth_vectex[0]+.5f), int(depth_vectex[1]+.5f))[0];
        float shadow = 1.f;
        int bias = 1;
        // std::cout << d << std::endl;
        if (depth_vectex[2] < d-bias) {
            // std::cout << shadow << std::endl;
            shadow = .3f;
        }
        
        // Phong + Shadow
        for (int i=0; i<3; i++) {
            // 5. : 环境光部分
            // 1. : 漫反射部分
            // .6 : 高光部分
            color[i] = std::min<float>(5. + c[i]*shadow*(1.*diff + .6*spec), 255);
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
    TGAImage depthmap(width, height, TGAImage::GRAYSCALE);

    // Depth Shading
    DepthShader depthShader;
    depthShader.mat_model = GetModelMatrix();
    depthShader.mat_view = GetViewMatrix(light1_dir, origin, up);
    depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    depthShader.computeTranform();
    shading(depthmap, zbuffer, depthShader);

    // Phong Shading
    // PhongShader phongShader;
    // phongShader.light = light1_dir;
    // phongShader.mat_model = GetModelMatrix();
    // phongShader.mat_view = GetViewMatrix(camera, origin, up);
    // phongShader.mat_project = GetProjectMatrix(camera, origin);
    // phongShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    // phongShader.computeTranform();
    // zbuffer.clear();
    // shading(image, zbuffer, phongShader);

    // Shadow Shading
    ShadowShader shadowShader;
    shadowShader.light = light1_dir;
    shadowShader.mat_model = GetModelMatrix();
    shadowShader.mat_view = GetViewMatrix(camera, origin, up);
    shadowShader.mat_project = GetProjectMatrix(camera, origin);
    shadowShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    shadowShader.mat_transform_depth = depthShader.mat_transform;
    shadowShader.depthmap = &depthmap;
    shadowShader.computeTranform();
    zbuffer.clear();
    shading(image, zbuffer, shadowShader);


    // the origin is at the left top cornor of image.
    image.flip_vertically();
    image.write_tga_file("output.tga");
    // zbuffer.flip_vertically();
    // zbuffer.write_tga_file("zbuffer.tga");
    // depthmap.flip_vertically();
    // depthmap.write_tga_file("depth.tga");
}

int main(int argc, char** argv) {

    model = new Model("../obj/african_head.obj");

    start();

    delete model;
    return 0;
}