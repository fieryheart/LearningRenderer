#include <vector>
#include <iostream>
#include "qgl.h"

Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth = 255;  // 根据深度值 计算颜色
const Vec3f light1_dir = Vec3f(1,-1,1).normalize();
const Vec3f camera(1,1,10);
const Vec3f origin(0,0,0);
const Vec3f up(0,1,0);
Matrix ModelMatrix(4,4);       // 模型空间
Matrix ViewMatrix(4,4);        // 视角空间
Matrix ProjectMatrix(4,4);     // 投影空间
Matrix ViewportMatrix(4,4);    // 屏幕空间


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
    // Model *model;
    Vec3f varying_intensity;
    // GouraudShader (Model *model_) : model(_model) {}
public:
    virtual Vec3f vertex(int nthface, int nthvert) {
        varying_intensity[nthvert] = std::max(0.f, model->norm(nthface, nthvert)*light1_dir);
        Vec3f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = mat2vec(
                    ViewportMatrix*ProjectMatrix*ViewMatrix*ModelMatrix*vec2mat(gl_vertex)
                );
        // std::cout << gl_vertex << std::endl;s
        return  gl_vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};


void start() {
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage texture;
    texture.read_tga_file("../obj/african_head_diffuse.tga");

    // 初始化各转换矩阵
    ModelMatrix = GetModelMatrix();
    ViewMatrix = GetViewMatrix(camera, origin, up);
    ProjectMatrix = GetProjectMatrix(camera, origin);
    ViewportMatrix = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);

    // std::cout << ModelMatrix << std::endl;
    // std::cout << ViewMatrix << std::endl;
    // std::cout << ProjectMatrix << std::endl;
    // std::cout << ViewportMatrix << std::endl;

    // Shading
    GouraudShader gouraudshader;
    for (int i=0; i<model->nfaces(); i++) {
        Vec3f screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = gouraudshader.vertex(i, j);
            // std::cout << screen_coords[j] << std::endl;
        }
        triangle(screen_coords, gouraudshader, image, zbuffer);
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