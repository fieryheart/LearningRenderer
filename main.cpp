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


// v 表示点
Matrix vec2mat(Vec3f v) {
    Matrix mat(4, 1);
    mat[0][0] = v[0];
    mat[1][0] = v[1];
    mat[2][0] = v[2];
    mat[3][0] = 1.f;
    return mat;
}

Vec3f mat2vec(Matrix mat) {
    Vec3f v;
    v[0] = mat[0][0];
    v[1] = mat[1][0];
    v[2] = mat[2][0];
    return v;
}

// 从texture中获取颜色
TGAColor getColorFromTexture(TGAImage &texture, Vec3f pos) {
    int tWidth = texture.get_width();
    int tHeight = texture.get_height();
    return texture.get(int(pos.x*tWidth), int(pos.y*tHeight));
} 


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
    // Vec3f varying_intensity;
    Vec3f varying_uv[3];
    Matrix uniform_M;
    Matrix uniform_MIT;

    virtual Vec4f vertex(int nthface, int nthvert) {
        // varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light1_dir, 0.f, 1.f);
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = TransformMatrix*gl_vertex;
        varying_uv[nthvert] = model->texCoord(nthface, nthvert);
        return  gl_vertex/gl_vertex[3];
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        // float intensity = varying_intensity*bar;
        Vec3f uv = varying_uv[0]*bar[0]+varying_uv[1]*bar[1]+varying_uv[2]*bar[2];
        Vec4f n(model->normal(uv),0.f);
        n = (uniform_M*n).normalize();
        Vec4f l(light1_dir, 0.f);
        l = (uniform_MIT*l).normalize();

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

void start() {
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    // TGAImage texture;
    // texture.read_tga_file("../obj/african_head_diffuse.tga");

    // 初始化各转换矩阵
    ModelMatrix = GetModelMatrix();
    ViewMatrix = GetViewMatrix(camera, origin, up);
    ProjectMatrix = GetProjectMatrix(camera, origin);
    ViewportMatrix = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    TransformMatrix = ViewportMatrix*ProjectMatrix*ViewMatrix*ModelMatrix;

    // std::cout << ModelMatrix << std::endl;
    // std::cout << ViewMatrix << std::endl;
    // std::cout << ProjectMatrix << std::endl;
    // std::cout << ViewportMatrix << std::endl;

    // Shading
    GouraudShader gouraudshader;
    ToonShader toonShader;
    PhongShader phongShader;

    phongShader.uniform_M = ProjectMatrix*ViewMatrix;
    phongShader.uniform_MIT = (ProjectMatrix*ViewMatrix).inverse().transpose();
    for (int i=0; i<model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = phongShader.vertex(i, j);
            // std::cout << screen_coords[j] << std::endl;
        }
        triangle(screen_coords, phongShader, image, zbuffer);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    zbuffer.flip_vertically();
    zbuffer.write_tga_file("zbuffer.tga");
}

int main(int argc, char** argv) {

    model = new Model("../obj/african_head.obj");

    start();

    delete model;
    return 0;
}