#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include "shaders.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth = 255;  // 根据深度值 计算颜色
const Vec3f light1_dir(1,1,1);
const Vec3f camera(1,1,10);
const Vec3f origin(0,0,0);
// const Vec3f up(0,1,0);

struct Log {
    bool flag;
    std::string prefix;
    Log(bool _flag, std::string _prefix) : flag(_flag), prefix(_prefix){}
};

// 在指定半径的半球上产生随机点
// 参考链接: https://mathworld.wolfram.com/SpherePointPicking.html
Vec3f RandInHemisphere() {
    float u = (float)rand()/(float)RAND_MAX;
    float v = (float)rand()/(float)RAND_MAX;
    float theta = 2.f*M_PI*u;
    float phi   = acos(2.f*v - 1.f);
    return Vec3f(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
}

void shading(TGAImage &image, float *zbuffer, Shader &shader, Log log) {
    Vec4f screen_coords[3];
    int nfaces = model->nfaces();
    for (int i=0; i<nfaces; i++) {
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        
        if (log.flag) {
            // std::cout << screen_coords[0] << std::endl;
            std::cout << "\r" << log.prefix << i*1.0/nfaces*100 << "% completed." << std::flush;
        }

        triangle(screen_coords, shader, image, zbuffer);
    }
    if (log.flag) {
        std::flush(std::cout);
        std::cout<< "\r" << log.prefix << "100% completed." << std::endl;
    }
}

// Implementation: Depth Shading
void DepthShading() {
    TGAImage image(width, height, TGAImage::GRAYSCALE);
    // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    float *zbuffer = new float[width*height];
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

    Vec3f light = light1_dir;
    Vec3f up(0, 1, 0);

    DepthShader depthShader;
    depthShader.model = model;
    depthShader.mat_model = GetModelMatrix();
    depthShader.mat_view = GetViewMatrix(light, origin, up);
    depthShader.mat_project = Matrix::identity(4);
    depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    depthShader.computeTranform();
    shading(image, zbuffer, depthShader, Log(false, ""));

    // save
    image.flip_vertically();    // the origin is at the left top cornor of image.
    image.write_tga_file("output_depth.tga");
    // zbuffer.flip_vertically();
    // zbuffer.write_tga_file("zbuffer.tga");
    delete[] zbuffer;
}

// Implementation: Phong Shading with no shadow
void PhongShadingNoShadow() {
    TGAImage image(width, height, TGAImage::RGB);
    float *zbuffer = new float[width*height];
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

    Vec3f light = light1_dir;
    Vec3f up(0, 1, 0);

    PhongShader phongShader;
    phongShader.model = model;
    phongShader.light = light;
    phongShader.mat_model = GetModelMatrix();
    phongShader.mat_view = GetViewMatrix(camera, origin, up);
    phongShader.mat_project = GetProjectMatrix(camera, origin);
    phongShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    phongShader.computeTranform();
    // zbuffer.clear();
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
    shading(image, zbuffer, phongShader, Log(false, ""));

    // save
    image.flip_vertically();    // the origin is at the left top cornor of image.
    image.write_tga_file("output_depth.tga");
    // zbuffer.flip_vertically();
    // zbuffer.write_tga_file("zbuffer.tga"); 
    delete[] zbuffer;
}

// Implementation: Phong Shading with hard shadow
void PhongShadingHardShadow() {
    TGAImage image(width, height, TGAImage::RGB);
    // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage depthmap(width, height, TGAImage::GRAYSCALE);
    float *zbuffer = new float[width*height];
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

    Vec3f light = light1_dir;
    Vec3f up(0, 1, 0);

    // Depth Shading
    DepthShader depthShader;
    depthShader.model = model;
    depthShader.mat_model = GetModelMatrix();
    depthShader.mat_view = GetViewMatrix(light, origin, up);
    depthShader.mat_project = Matrix::identity(4);
    depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    depthShader.computeTranform();
    shading(depthmap, zbuffer, depthShader, Log(false, "")); 

    // Shadow Shading
    ShadowShader shadowShader;
    shadowShader.model = model;
    shadowShader.light = light;
    shadowShader.mat_model = GetModelMatrix();
    shadowShader.mat_view = GetViewMatrix(camera, origin, up);
    shadowShader.mat_project = GetProjectMatrix(camera, origin);
    shadowShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
    shadowShader.mat_transform_depth = depthShader.mat_transform;
    shadowShader.depthmap = &depthmap;
    shadowShader.computeTranform();
    // zbuffer.clear();
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
    shading(image, zbuffer, shadowShader, Log(false, ""));

    // save
    image.flip_vertically();    // the origin is at the left top cornor of image.
    image.write_tga_file("output.tga");
    // zbuffer.flip_vertically();
    // zbuffer.write_tga_file("zbuffer.tga");
    depthmap.flip_vertically();
    depthmap.write_tga_file("depth.tga");

    delete[] zbuffer; 
}

// Implementation: AO, Ambient Occlusion
void AmbientOcclusion() {

    srand((unsigned)time(NULL));
 
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage depthmap(width, height, TGAImage::RGB);
    // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage occlusion(model->getDiffuseMapWidth(), model->getDiffuseMapHeight(), TGAImage::GRAYSCALE);
    std::vector<std::vector<int>> total(model->getDiffuseMapWidth(), std::vector<int>(model->getDiffuseMapHeight(), 0));
    float *zbuffer = new float[width*height];
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

    int sampleNum = 1;

    for (int i = 0; i < sampleNum; ++i) {
        std::cout << "The " << i << "th Ambient Occlusion rendering: " << std::endl;

        Vec3f light = RandInHemisphere();
        light.y = std::abs(light.y);
        Vec3f up;
        for (int i = 0; i < 3; ++i) up[i] = (float)rand()/(float)RAND_MAX;
        std::cout << "light: " << light << std::endl;

        image.clear();
        depthmap.clear();
        occlusion.clear();

        // The first pass
        // 生成深度图
        // zbuffer.clear();
        std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
        DepthShader depthShader;
        depthShader.model = model;
        depthShader.mat_model = GetModelMatrix();
        depthShader.mat_view = GetViewMatrix(light, origin, up);
        depthShader.mat_project = Matrix::identity(4);
        depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
        depthShader.computeTranform();

        shading(depthmap, zbuffer, depthShader, Log(true, "Depth Shading: "));

        // The second pass
        // 在occlusion上着点
        OcclusionShader occlusionShader;
        occlusionShader.model = model;
        occlusionShader.mat_model = GetModelMatrix();
        occlusionShader.mat_view = GetViewMatrix(light, origin, up);
        occlusionShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
        occlusionShader.computeTranform();
        occlusionShader.depthmap = &depthmap;
        occlusionShader.occlusionmap = &occlusion;
        // zbuffer.clear();
        std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
        shading(image, zbuffer, occlusionShader, Log(true, "Occlusion Shading: "));

        // 加入当前occlusion
        std::cout << "add to the occlusion" << std::endl;
        for (int i = 0; i < occlusion.get_width(); ++i) {
            for (int j = 0; j < occlusion.get_height(); ++j) {
                // if (occlusion.get(i, j)[0]) {
                //     std::cout << (int)occlusion.get(i, j)[0] << std::endl;
                // }
                total[i][j] += (int)occlusion.get(i, j)[0];
            }
        }

        std::cout << std::endl;
    }

    // average the occlusion image
    std::cout << "average the occlusion image" << std::endl;
    for (int i = 0; i < total.size(); ++i) {
        for (int j = 0; j < total[i].size(); ++j) {
            // if (total[i][j]) {
            //     std::cout << total[i][j] << std::endl;
            // }
            occlusion.set(i, j, int(total[i][j]*1.0/sampleNum + .5));
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    occlusion.flip_vertically();
    occlusion.write_tga_file("occlusion.tga");
    depthmap.flip_vertically();
    depthmap.write_tga_file("occlusion_depth.tga");

    delete[] zbuffer;
}

float _get_max_slope(float *zbuffer, Vec2f pos, Vec2f dir) {
    assert(width == height);
    float max_slope = 0.0f;
    for (float t = 0; t < 1000.f; t += 1.0f) {
        Vec2f p = pos + dir*t;
        if (p.x >= width || p.y >= height || p.x < 0 || p.y < 0) return max_slope;
        float dis = t;
        if (dis < 1.f) continue;

        float dh = zbuffer[int(p.x)+width*int(p.y)] - zbuffer[int(pos.x)+width*int(pos.y)];
        // dh /= 255;
        max_slope = std::max(max_slope, atanf(dh/dis));
    }
    return max_slope;
}

// Implementation: SSAO, Screen Space Ambient Occlusion
// zbuffer 不能使用int存储，不然会出现明显的分割纹路
void ScreenSpaceAmbientOcclusion() {
    TGAImage image(width, height, TGAImage::GRAYSCALE);
    TGAImage depthmap(width, height, TGAImage::GRAYSCALE);
    // std::vector<std::vector<float>> zb(width, std::vector<float>(height, 0.0f));
    float *zbuffer = new float[width*height];
    std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
    
    // depth shading
    Vec3f light = light1_dir;
    Vec3f up(0, 1, 0);

    DepthShader depthShader;
    depthShader.model = model;
    depthShader.mat_model = GetModelMatrix();
    depthShader.mat_view = GetViewMatrix(camera, origin, up);
    // depthShader.mat_project = Matrix::identity(4);
    depthShader.mat_project = GetProjectMatrix(camera, origin);
    depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, 1);
    depthShader.computeTranform();
    shading(image, zbuffer, depthShader, Log(true, "Depth Shading: "));

    // post-processing
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (zbuffer[i+width*j] < 1e-5) continue;
            depthmap.set(i, j, (int)(zbuffer[i+width*j]*255));
            // zbuffer[i+width*j] = (int)(zbuffer[i+width*j]*255);
        }
    }
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (zbuffer[i+width*j] < 1e-5) continue;
            float total_slope = 0.0f;

            // case 1e-4 !!! 
            // 不然会多算一次
            for (float dir = 0; dir < 2*M_PI-1e-4; dir += M_PI/4) {
                total_slope += M_PI/2 - _get_max_slope(zbuffer, Vec2f(i, j), Vec2f(sin(dir), cos(dir)));
            }
            // for (float dir = 0; dir < 2*M_PI; dir += M_PI/4) {
            //     total_slope += M_PI/2 - _get_max_slope(zbuffer, Vec2f(i, j), Vec2f(sin(dir), cos(dir)));
            // }
            total_slope /= (M_PI/2)*8;

            total_slope = pow(total_slope, 100.f);  // image will be hard to analysis if not do this.

            image.set(i, j, total_slope*255);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output_SSAO.tga");
    depthmap.flip_vertically();
    depthmap.write_tga_file("output_depth.tga");

    delete[] zbuffer;
}


// generate an image
void renderAImage() {
    const int width = 1024;
    const int height = 762;
    std::vector<Vec3f> framebuffer(width*height);

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            framebuffer[i+j*width] = Vec3f(j*1.0f/height, i*1.0f/width, 0);
        }
    }

    std::string filename = "./out.png";
    // std::ofstream ofs;
    // ofs.open("./out.png");
    auto data = (unsigned char*)malloc(width*height*3);
    for (int i = 0; i < width*height; ++i) {
        for (int j = 0; j < 3; ++j) {
            data[i*3+j] = (unsigned char)(255*std::max(0.0f, std::min(1.0f, framebuffer[i][j])));
        }
    }
    
    stbi_write_png(filename.c_str(), width, height, 3, data, 0);

    // ofs.close();
}

int main(int argc, char** argv) {

    renderAImage();

    // model = new Model("../obj/african_head.obj");

    // ScreenSpaceAmbientOcclusion();

    // delete model;
    return 0;
}