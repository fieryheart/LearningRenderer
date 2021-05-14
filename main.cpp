#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <omp.h>
// #include "shaders.h"
#include "qgl.h"

const int width  = 800;
const int height = 800;
const int depth = 1;  // 根据深度值 计算颜色

// 画个三角形
/*
void Example01() {
    const Vec3f light1_dir(1,1,1);
    const Vec3f camera(0,0,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    QGL::Frame frame = QGL::Frame(width, height);

    Vec3f A = Vec3f(100.0f, 0.0f, 0.0f);
    Vec3f B = Vec3f(-100.0f, 0.0f, 0.0f);
    Vec3f C = Vec3f(0.0f, 100.0f, 0.0f);
    std::vector<Vec3f> verts{A, B, C};
    
    QGL::Face face = QGL::Face(Vec3i(0, 1, 2));
    std::vector<QGL::Face> faces{face};

    QGL::Model *model = new QGL::Model(verts, faces);

    std::cout << "set camera." << std::endl;
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    QGL::SetOrthogonalProjectMat(width, height, depth);
    QGL::SetScreenMat(0, 0, width, height, depth);
    QGL::SetCamera(false);
    std::cout << "QGL::MAT_OPROJECT" << std::endl;
    std::cout << QGL::MAT_OPROJECT << std::endl;

    std::cout << "set testShader." << std::endl;
    QGL::TestShader testShader = QGL::TestShader();
    testShader.uniform_mat_transform = QGL::MAT_TRANS;

    std::cout << "do Rendering." << std::endl;
    // QGL::Rendering(model, testShader, frame);

    std::cout << "draw frame." << std::endl;
    std::string result = "../example/out.png";
    QGL::FlipFrame(frame);
    QGL::DrawFrame(frame, result.c_str());

    delete model;
}
*/

// 加载obj模型文件
/*
void Example02() {
    const Vec3f light(1,1,1);
    const Vec3f camera(0,0,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    QGL::Frame frame = QGL::Frame(width, height);
    QGL::Zbuffer zbuffer = QGL::Zbuffer(width, height);
    QGL::Log log = QGL::Log(true, "Depth Shading: ");
    

    QGL::Model *model = new QGL::Model("../obj/Marry.obj");
    model->vertsNormalize();

    // Camera
    std::cout << "set camera." << std::endl;
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    // QGL::SetOrthogonalProjectMat(width, height, depth);
    QGL::SetScreenMat(-100, -350, 1000, 1000, depth);
    QGL::SetCamera(true);

    // Shader
    std::cout << "set DepthShader." << std::endl;
    QGL::DepthShader depthShader = QGL::DepthShader();
    depthShader.uniform_mat_transform = QGL::MAT_TRANS;    

    // Render
    QGL::RenderNode rn;
    rn.model = model;
    rn.shader = &depthShader;
    rn.frame = &frame;
    // rn.zbuffer = &zbuffer;
    rn.log = &log;
    std::cout << "do Rendering." << std::endl;
    QGL::Rendering(rn);

    std::cout << "draw frame." << std::endl;
    std::string result = "../example/out.png";
    QGL::FlipFrame(frame);
    QGL::DrawFrame(frame, result.c_str());

    delete model;
}
*/

// 加载纹理图片
void Example03() {
    const Vec3f light(1,1,1);
    const Vec3f camera(1,1,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    QGL::Frame frame = QGL::Frame(width, height, 4);
    QGL::Zbuffer zbuffer = QGL::Zbuffer(width, height);
    QGL::Log log = QGL::Log(true, "Texture Shading: ");
    QGL::NUMTHREADS = 8;
 
    QGL::Model *model = new QGL::Model("../obj/Marry.obj");
    model->vertsNormalize();
    model->loadMap("../obj/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);

    // Camera
    std::cout << "set camera." << std::endl;
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    // QGL::SetOrthogonalProjectMat(width, height, depth);
    QGL::SetScreenMat(-100, -350, 1000, 1000, depth);
    QGL::SetCamera(true);

    // Shader
    std::cout << "set TexShader." << std::endl;
    QGL::TexShader texShader = QGL::TexShader();
    texShader.uniform_mat_transform = QGL::MAT_TRANS;

    // Render
    QGL::RenderNode rn;
    rn.model = model;
    rn.shader = &texShader;
    rn.frame = &frame;
    rn.zbuffer = &zbuffer;
    rn.log = &log;
    rn.comType = QGL::CT_Single;
    std::cout << "do Rendering." << std::endl;
    QGL::Timer timer = QGL::Timer();
    timer.update();
    QGL::Rendering(rn);
    std::cout << timer.millisec() << "ms" << std::endl;

    std::cout << "draw frame." << std::endl;
    std::string out = "../example/out.png";
    frame.flip();
    frame.draw(out.c_str());
    // QGL::FlipFrame(frame);
    // QGL::DrawFrame(frame, result.c_str());


    delete model;
    // delete[] zbuffer;
}

// Gouraud Shader
/*
void Example04() {
    const Vec3f camera(1,1,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    Vec3f light(1,1,1);
    QGL::Frame frame = QGL::Frame(width, height, 4);
    QGL::Zbuffer zbuffer = QGL::Zbuffer(width, height);
    QGL::Log log = QGL::Log(true, "Gouraud Shading: ");
    // QGL::NUMTHREADS = 8;
 
    QGL::Model *model = new QGL::Model("../obj/Marry.obj");
    model->vertsNormalize();
    model->loadMap("../obj/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);

    // Camera
    std::cout << "set camera." << std::endl;
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    // QGL::SetOrthogonalProjectMat(width, height, depth);
    QGL::SetScreenMat(-100, -350, 1000, 1000, depth);
    QGL::SetCamera(true);

    // Shader
    std::cout << "set GouraudShader." << std::endl;
    QGL::GouraudShader gouraudShader = QGL::GouraudShader();
    gouraudShader.uniform_mat_transform = QGL::MAT_TRANS;
    gouraudShader.uniform_mat_norm_transform = QGL::MAT_NORM_TRANS;

    gouraudShader.uniform_light = light.normalize();

    // Render
    QGL::RenderNode rn;
    rn.model = model;
    rn.shader = &gouraudShader;
    rn.frame = &frame;
    rn.zbuffer = &zbuffer;
    rn.log = &log;
    rn.comType = QGL::CT_Single;
    std::cout << "do Rendering." << std::endl;
    QGL::Timer timer = QGL::Timer();
    timer.update();
    QGL::Rendering(rn);
    std::cout << timer.second() << "s" << std::endl;

    std::cout << "draw frame." << std::endl;
    std::string out = "../example/out.png";
    frame.flip();
    frame.draw(out.c_str());

    delete model;
}
*/

// Phong Shader with no shadow
/*
void Example05() {
    const Vec3f camera(1,1,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    Vec3f light(0,1,0);
    QGL::Frame frame = QGL::Frame(width, height, 4);
    QGL::Zbuffer zbuffer = QGL::Zbuffer(width, height);
    QGL::Log log = QGL::Log(true, "Gouraud Shading: ");
    // QGL::NUMTHREADS = 8;
 
    QGL::Model *model = new QGL::Model("../obj/Marry.obj");
    model->vertsNormalize();
    model->loadMap("../obj/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);

    // Camera
    std::cout << "set camera." << std::endl;
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    // QGL::SetOrthogonalProjectMat(width, height, depth);
    QGL::SetScreenMat(-100, -350, 1000, 1000, depth);
    QGL::SetCamera(true);

    // Shader
    std::cout << "set PhongShader." << std::endl;
    QGL::PhongShader phongShader = QGL::PhongShader();
    phongShader.uniform_mat_transform = QGL::MAT_TRANS;
    phongShader.uniform_mat_norm_transform = QGL::MAT_NORM_TRANS;
    phongShader.uniform_camera = camera;
    phongShader.uniform_light = light.normalize();

    // Render
    QGL::RenderNode rn;
    rn.model = model;
    rn.shader = &phongShader;
    rn.frame = &frame;
    rn.zbuffer = &zbuffer;
    rn.log = &log;
    rn.comType = QGL::CT_Single;
    std::cout << "do Rendering." << std::endl;
    QGL::Timer timer = QGL::Timer();
    timer.update();
    QGL::Rendering(rn);
    std::cout << timer.second() << "s" << std::endl;

    std::cout << "draw frame." << std::endl;
    std::string out = "../example/out.png";
    frame.flip();
    frame.draw(out.c_str());

    delete model;
}
*/

// Phong Shader with hard shadow
void Example06() {
    const Vec3f camera(1,1,1);
    const Vec3f origin(0,0,0);
    const Vec3f up(0,1,0);
    QGL::DirectLight dl = QGL::DirectLight(Vec3f(0, 1, 1), Vec3f(0,-1,-1));
    QGL::Frame depthFrame = QGL::Frame(width, height, 4);
    depthFrame.id = 1;
    QGL::Frame frame = QGL::Frame(width, height, 4);
    frame.id = 2;

    QGL::Zbuffer zbuffer = QGL::Zbuffer(width, height);
    QGL::RenderNode rn;
    QGL::Log log = QGL::Log(true, "Depth Shading: ");
    QGL::Timer timer = QGL::Timer();
    // QGL::NUMTHREADS = 8;
 
    // Marry Model
    QGL::Model *marry = new QGL::Model("../obj/Marry.obj");
    marry->vertsNormalize();
    marry->loadMap("../obj/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);

    // Plane Model
    Vec3f pts[4] = {Vec3f(-0.7, 0, 0.5), Vec3f(-0.7, 0, -0.8),
                    Vec3f(0.7, 0, 0.5),  Vec3f(0.7, 0, -0.8)};
    QGL::Plane plane = QGL::Plane(pts);
    QGL::Model *planeModel = new QGL::Model(plane);


    // Set camera from light
    timer.update();
    log = QGL::Log(true, "Depth Shading: ");

    QGL::SetModelMat();
    QGL::SetViewMat(dl.pos, origin, up);
    // QGL::SetPerspectiveProjectMat(camera, origin);
    QGL::SetScreenMat(0, 0, 800, 800, depth);
    QGL::SetCamera(true);

    // Depth Shader
    QGL::DepthShader depthShader = QGL::DepthShader();
    depthShader.uniform_mat_transform = QGL::MAT_TRANS;
    
    // 
    rn.shader = &depthShader;
    rn.frame = &depthFrame;
    rn.zbuffer = &zbuffer;
    rn.log = &log;
    rn.comType = QGL::CT_Single;
    //
    rn.model = marry;
    QGL::Rendering(rn);
    // 
    rn.model = planeModel;
    QGL::Rendering(rn);

    std::cout << timer.second() << "s" << std::endl;


    // Marry && Plane
    timer.update();

    QGL::PlaneShader planeShader = QGL::PlaneShader();
    QGL::ShadowShader shadowShader = QGL::ShadowShader();
    planeShader.uniform_mat_depth_transform = QGL::MAT_TRANS;
    shadowShader.uniform_mat_depth_transform = QGL::MAT_TRANS;
    planeShader.depthFrame = &depthFrame;
    shadowShader.depthFrame = &depthFrame;
    // 
    rn.frame = &frame;
    zbuffer.clear();
    //
    QGL::SetModelMat();
    QGL::SetViewMat(camera, origin, up);
    QGL::SetPerspectiveProjectMat(camera, origin);
    QGL::SetScreenMat(0, 0, 800, 800, depth);
    QGL::SetCamera(true);

    // Marry
    log = QGL::Log(true, "Shadow Shading: ");

    shadowShader.uniform_mat_transform = QGL::MAT_TRANS;
    shadowShader.uniform_mat_transform_i = (QGL::MAT_TRANS).inverse(); 
    shadowShader.uniform_mat_norm_transform = QGL::MAT_NORM_TRANS;
    shadowShader.uniform_camera = camera;
    shadowShader.uniform_light = -dl.dir;
    
    rn.shader = &shadowShader;
    rn.model = marry;
    QGL::Rendering(rn);

    // Plane
    log = QGL::Log(true, "Plane Shading: ");
    planeShader.uniform_mat_transform = QGL::MAT_TRANS;
    planeShader.uniform_mat_transform_i = (QGL::MAT_TRANS).inverse();
    planeShader.uniform_color = plane.color;

    rn.shader = &planeShader;
    rn.model = planeModel;
    QGL::Rendering(rn);

    std::cout << timer.second() << "s" << std::endl;

    std::cout << "draw depth frame." << std::endl;
    std::string out_depth = "../example/out_depth.png";
    depthFrame.flip();
    depthFrame.draw(out_depth.c_str());

    std::cout << "draw frame." << std::endl;
    std::string out = "../example/out.png";
    frame.flip();
    frame.draw(out.c_str());

    // delete planeModel;
    delete marry;
}

void ExampleOmp() {
    // std::cout << "Num theads supported: " << omp_get_num_procs() << std::endl;
	// omp_set_num_threads(4);
    // int num_procs = omp_get_num_procs();
    // std::cout << "Num theads supported: " << num_procs << std::endl;
    // #pragma omp parallel num_threads(2*num_procs-1)
    #pragma omp parallel
	{
		std::cout << "Hello" << ", I am Thread " << omp_get_thread_num() << std::endl;
	}
}

int main(int argc, char** argv) {

    Example06();

    return 0;
}


// struct Log {
//     bool flag;
//     std::string prefix;
//     Log(bool _flag, std::string _prefix) : flag(_flag), prefix(_prefix){}
// };

// // 在指定半径的半球上产生随机点
// // 参考链接: https://mathworld.wolfram.com/SpherePointPicking.html
// Vec3f RandInHemisphere() {
//     float u = (float)rand()/(float)RAND_MAX;
//     float v = (float)rand()/(float)RAND_MAX;
//     float theta = 2.f*M_PI*u;
//     float phi   = acos(2.f*v - 1.f);
//     return Vec3f(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
// }

// void shading(TGAImage &image, float *zbuffer, Shader &shader, Log log) {
//     Vec4f screen_coords[3];
//     int nfaces = model->nfaces();
//     for (int i=0; i<nfaces; i++) {
//         for (int j=0; j<3; j++) {
//             screen_coords[j] = shader.vertex(i, j);
//         }
        
//         if (log.flag) {
//             // std::cout << screen_coords[0] << std::endl;
//             std::cout << "\r" << log.prefix << i*1.0/nfaces*100 << "% completed." << std::flush;
//         }

//         triangle(screen_coords, shader, image, zbuffer);
//     }
//     if (log.flag) {
//         std::flush(std::cout);
//         std::cout<< "\r" << log.prefix << "100% completed." << std::endl;
//     }
// }

// // Implementation: Depth Shading
// void DepthShading() {
//     TGAImage image(width, height, TGAImage::GRAYSCALE);
//     // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
//     float *zbuffer = new float[width*height];
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

//     Vec3f light = light1_dir;
//     Vec3f up(0, 1, 0);

//     DepthShader depthShader;
//     depthShader.model = model;
//     depthShader.mat_model = GetModelMatrix();
//     depthShader.mat_view = GetViewMatrix(light, origin, up);
//     depthShader.mat_project = Matrix::identity(4);
//     depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//     depthShader.computeTranform();
//     shading(image, zbuffer, depthShader, Log(false, ""));

//     // save
//     image.flip_vertically();    // the origin is at the left top cornor of image.
//     image.write_tga_file("output_depth.tga");
//     // zbuffer.flip_vertically();
//     // zbuffer.write_tga_file("zbuffer.tga");
//     delete[] zbuffer;
// }

// // Implementation: Phong Shading with no shadow
// void PhongShadingNoShadow() {
//     TGAImage image(width, height, TGAImage::RGB);
//     float *zbuffer = new float[width*height];
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

//     Vec3f light = light1_dir;
//     Vec3f up(0, 1, 0);

//     PhongShader phongShader;
//     phongShader.model = model;
//     phongShader.light = light;
//     phongShader.mat_model = GetModelMatrix();
//     phongShader.mat_view = GetViewMatrix(camera, origin, up);
//     phongShader.mat_project = GetProjectMatrix(camera, origin);
//     phongShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//     phongShader.computeTranform();
//     // zbuffer.clear();
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
//     shading(image, zbuffer, phongShader, Log(false, ""));

//     // save
//     image.flip_vertically();    // the origin is at the left top cornor of image.
//     image.write_tga_file("output_depth.tga");
//     // zbuffer.flip_vertically();
//     // zbuffer.write_tga_file("zbuffer.tga"); 
//     delete[] zbuffer;
// }

// // Implementation: Phong Shading with hard shadow
// void PhongShadingHardShadow() {
//     TGAImage image(width, height, TGAImage::RGB);
//     // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
//     TGAImage depthmap(width, height, TGAImage::GRAYSCALE);
//     float *zbuffer = new float[width*height];
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

//     Vec3f light = light1_dir;
//     Vec3f up(0, 1, 0);

//     // Depth Shading
//     DepthShader depthShader;
//     depthShader.model = model;
//     depthShader.mat_model = GetModelMatrix();
//     depthShader.mat_view = GetViewMatrix(light, origin, up);
//     depthShader.mat_project = Matrix::identity(4);
//     depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//     depthShader.computeTranform();
//     shading(depthmap, zbuffer, depthShader, Log(false, "")); 

//     // Shadow Shading
//     ShadowShader shadowShader;
//     shadowShader.model = model;
//     shadowShader.light = light;
//     shadowShader.mat_model = GetModelMatrix();
//     shadowShader.mat_view = GetViewMatrix(camera, origin, up);
//     shadowShader.mat_project = GetProjectMatrix(camera, origin);
//     shadowShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//     shadowShader.mat_transform_depth = depthShader.mat_transform;
//     shadowShader.depthmap = &depthmap;
//     shadowShader.computeTranform();
//     // zbuffer.clear();
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
//     shading(image, zbuffer, shadowShader, Log(false, ""));

//     // save
//     image.flip_vertically();    // the origin is at the left top cornor of image.
//     image.write_tga_file("output.tga");
//     // zbuffer.flip_vertically();
//     // zbuffer.write_tga_file("zbuffer.tga");
//     depthmap.flip_vertically();
//     depthmap.write_tga_file("depth.tga");

//     delete[] zbuffer; 
// }

// // Implementation: AO, Ambient Occlusion
// void AmbientOcclusion() {

//     srand((unsigned)time(NULL));
 
//     TGAImage image(width, height, TGAImage::RGB);
//     TGAImage depthmap(width, height, TGAImage::RGB);
//     // TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
//     TGAImage occlusion(model->getDiffuseMapWidth(), model->getDiffuseMapHeight(), TGAImage::GRAYSCALE);
//     std::vector<std::vector<int>> total(model->getDiffuseMapWidth(), std::vector<int>(model->getDiffuseMapHeight(), 0));
//     float *zbuffer = new float[width*height];
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());

//     int sampleNum = 1;

//     for (int i = 0; i < sampleNum; ++i) {
//         std::cout << "The " << i << "th Ambient Occlusion rendering: " << std::endl;

//         Vec3f light = RandInHemisphere();
//         light.y = std::abs(light.y);
//         Vec3f up;
//         for (int i = 0; i < 3; ++i) up[i] = (float)rand()/(float)RAND_MAX;
//         std::cout << "light: " << light << std::endl;

//         image.clear();
//         depthmap.clear();
//         occlusion.clear();

//         // The first pass
//         // 生成深度图
//         // zbuffer.clear();
//         std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
//         DepthShader depthShader;
//         depthShader.model = model;
//         depthShader.mat_model = GetModelMatrix();
//         depthShader.mat_view = GetViewMatrix(light, origin, up);
//         depthShader.mat_project = Matrix::identity(4);
//         depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//         depthShader.computeTranform();

//         shading(depthmap, zbuffer, depthShader, Log(true, "Depth Shading: "));

//         // The second pass
//         // 在occlusion上着点
//         OcclusionShader occlusionShader;
//         occlusionShader.model = model;
//         occlusionShader.mat_model = GetModelMatrix();
//         occlusionShader.mat_view = GetViewMatrix(light, origin, up);
//         occlusionShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, depth);
//         occlusionShader.computeTranform();
//         occlusionShader.depthmap = &depthmap;
//         occlusionShader.occlusionmap = &occlusion;
//         // zbuffer.clear();
//         std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
//         shading(image, zbuffer, occlusionShader, Log(true, "Occlusion Shading: "));

//         // 加入当前occlusion
//         std::cout << "add to the occlusion" << std::endl;
//         for (int i = 0; i < occlusion.get_width(); ++i) {
//             for (int j = 0; j < occlusion.get_height(); ++j) {
//                 // if (occlusion.get(i, j)[0]) {
//                 //     std::cout << (int)occlusion.get(i, j)[0] << std::endl;
//                 // }
//                 total[i][j] += (int)occlusion.get(i, j)[0];
//             }
//         }

//         std::cout << std::endl;
//     }

//     // average the occlusion image
//     std::cout << "average the occlusion image" << std::endl;
//     for (int i = 0; i < (int)total.size(); ++i) {
//         for (int j = 0; j < (int)total[i].size(); ++j) {
//             // if (total[i][j]) {
//             //     std::cout << total[i][j] << std::endl;
//             // }
//             occlusion.set(i, j, int(total[i][j]*1.0/sampleNum + .5));
//         }
//     }

//     image.flip_vertically();
//     image.write_tga_file("output.tga");
//     occlusion.flip_vertically();
//     occlusion.write_tga_file("occlusion.tga");
//     depthmap.flip_vertically();
//     depthmap.write_tga_file("occlusion_depth.tga");

//     delete[] zbuffer;
// }

// float _get_max_slope(float *zbuffer, Vec2f pos, Vec2f dir) {
//     assert(width == height);
//     float max_slope = 0.0f;
//     for (float t = 0; t < 1000.f; t += 1.0f) {
//         Vec2f p = pos + dir*t;
//         if (p.x >= width || p.y >= height || p.x < 0 || p.y < 0) return max_slope;
//         float dis = t;
//         if (dis < 1.f) continue;

//         float dh = zbuffer[int(p.x)+width*int(p.y)] - zbuffer[int(pos.x)+width*int(pos.y)];
//         // dh /= 255;
//         max_slope = std::max(max_slope, atanf(dh/dis));
//     }
//     return max_slope;
// }

// // Implementation: SSAO, Screen Space Ambient Occlusion
// // zbuffer 不能使用int存储，不然会出现明显的分割纹路
// void ScreenSpaceAmbientOcclusion() {
//     TGAImage image(width, height, TGAImage::GRAYSCALE);
//     TGAImage depthmap(width, height, TGAImage::GRAYSCALE);
//     // std::vector<std::vector<float>> zb(width, std::vector<float>(height, 0.0f));
//     float *zbuffer = new float[width*height];
//     std::fill(zbuffer, zbuffer+width*height, -std::numeric_limits<float>::max());
    
//     // depth shading
//     Vec3f light = light1_dir;
//     Vec3f up(0, 1, 0);

//     DepthShader depthShader;
//     depthShader.model = model;
//     depthShader.mat_model = GetModelMatrix();
//     depthShader.mat_view = GetViewMatrix(camera, origin, up);
//     // depthShader.mat_project = Matrix::identity(4);
//     depthShader.mat_project = GetProjectMatrix(camera, origin);
//     depthShader.mat_viewport = GetViewportMatrix(width/8, height/8, width*3/4, width*3/4, 1);
//     depthShader.computeTranform();
//     shading(image, zbuffer, depthShader, Log(true, "Depth Shading: "));

//     // post-processing
//     for (int i = 0; i < width; ++i) {
//         for (int j = 0; j < height; ++j) {
//             if (zbuffer[i+width*j] < 1e-5) continue;
//             depthmap.set(i, j, (int)(zbuffer[i+width*j]*255));
//             // zbuffer[i+width*j] = (int)(zbuffer[i+width*j]*255);
//         }
//     }
//     for (int i = 0; i < width; ++i) {
//         for (int j = 0; j < height; ++j) {
//             if (zbuffer[i+width*j] < 1e-5) continue;
//             float total_slope = 0.0f;

//             // case 1e-4 !!! 
//             // 不然会多算一次
//             for (float dir = 0; dir < 2*M_PI-1e-4; dir += M_PI/4) {
//                 total_slope += M_PI/2 - _get_max_slope(zbuffer, Vec2f(i, j), Vec2f(sin(dir), cos(dir)));
//             }
//             // for (float dir = 0; dir < 2*M_PI; dir += M_PI/4) {
//             //     total_slope += M_PI/2 - _get_max_slope(zbuffer, Vec2f(i, j), Vec2f(sin(dir), cos(dir)));
//             // }
//             total_slope /= (M_PI/2)*8;

//             total_slope = pow(total_slope, 100.f);  // image will be hard to analysis if not do this.

//             image.set(i, j, total_slope*255);
//         }
//     }

//     image.flip_vertically();
//     image.write_tga_file("output_SSAO.tga");
//     depthmap.flip_vertically();
//     depthmap.write_tga_file("output_depth.tga");

//     delete[] zbuffer;
// }


// // generate an image
// void RenderAImage() {
//     const int width = 1024;
//     const int height = 762;
//     std::vector<Vec3f> framebuffer(width*height);

//     for (int j = 0; j < height; ++j) {
//         for (int i = 0; i < width; ++i) {
//             framebuffer[i+j*width] = Vec3f(j*1.0f/height, i*1.0f/width, 0);
//         }
//     }

//     std::string filename = "./out.png";
//     // std::ofstream ofs;
//     // ofs.open("./out.png");
//     auto data = (unsigned char*)malloc(width*height*3);
//     for (int i = 0; i < width*height; ++i) {
//         for (int j = 0; j < 3; ++j) {
//             data[i*3+j] = (unsigned char)(255*std::max(0.0f, std::min(1.0f, framebuffer[i][j])));
//         }
//     }
    
//     stbi_write_png(filename.c_str(), width, height, 3, data, 0);

//     // ofs.close();
// }

// // Sphere
// struct Sphere {
//     Vec3f center;
//     float radius;
//     Vec3f color;
//     int id;
//     int material;   // 0: phong; 1: +reflection; 2: +refraction

//     Sphere(Vec3f _center, float _radius, Vec3f _color, int _id, int _material) : 
//         center(_center), radius(_radius), color(_color), id(_id), material(_material) {}

//     /*              
//      *                        · center
//      *                       ··
//      *                     ····  
//      *               c   ······
//      *                 ········  b
//      *                ·········
//      *              ···········
//      *     origin ·············
//      *                   a   
//      */
//     bool intersect(Vec3f &origin, Vec3f &dir, float &t) const {
//         Vec3f vec_c = center - origin;
//         float a = vec_c*dir;
//         float b_2 = vec_c*vec_c - a*a;
//         if (b_2 > radius*radius) return false;

//         float d = radius*radius - b_2;
//         t = a - sqrtf(d);
//         if (t < 0) t = a + sqrt(d);
//         if (t < 0) return false;
//         return true;
//     }
// };

// // PointLight
// struct PointLight {
//     Vec3f pos;
//     float intensity;
//     PointLight(Vec3f pos, float intensity) : pos(pos), intensity(intensity) {}
// };

// // get the xyz-coordinates of the point
// Vec3f Coord(Vec3f &origin, Vec3f &dir, float t) {
//     return origin + dir*t;
// }

// Vec3f PhongRender(Vec3f &view, Vec3f &p, Vec3f &N, Vec3f color, std::vector<PointLight> lights) {
//     float ambient = 0.005f;
//     float diffuse = 0.0f;
//     float specular = 0.0f;
//     for (int i = 0; i < lights.size(); ++i) {
//         Vec3f dir = (lights[i].pos - p).normalize();
//         float beta = 1.0f;
//         beta = std::max(0.0f, dir*N);
//         diffuse += beta*lights[i].intensity/(lights[i].pos - p).norm();

//         // specular
//         Vec3f h = (view + dir).normalize();
//         float a = std::max(0.0f, h*N);
//         specular += std::pow(a, 100.0f)*lights[i].intensity/(lights[i].pos - p).norm();
//     }
//     return color * (1.0f * diffuse + 0.6 * specular) + ambient;
// }

// void shadowRender(Vec3f p, std::vector<Sphere> &spheres, std::vector<PointLight> &lights, std::vector<PointLight> &_lights) {
//     for (auto l : lights) {
//         Vec3f origin = p, dir = (l.pos - p).normalize();
//         float t = std::numeric_limits<float>::max();
//         float interFlag = false;
//         for (auto sphere : spheres) {
//             if (sphere.intersect(origin, dir, t)) {
//                 interFlag = true;
//                 break;
//             }
//         }
//         if (interFlag) continue;
//         _lights.push_back(l);
//     }
// }

// Vec3f reflect(Vec3f N, Vec3f in) {
//     return (N*(N*in*2.0f)-in).normalize();
// }

// Vec3f RayTracing(Vec3f &origin, Vec3f &dir, std::vector<Sphere> &spheres, std::vector<PointLight> &lights, int depth) {
//     float min_t = std::numeric_limits<float>::max();
//     Vec3f color(0.2, 0.7, 0.8);
//     if (depth >= 2) return color;
//     for (auto sphere : spheres) {
//         float t = std::numeric_limits<float>::max();
//         if (sphere.intersect(origin, dir, t) && t < min_t) {
//             min_t = t;
//             Vec3f p = Coord(origin, dir, min_t);
//             Vec3f N = (p - sphere.center).normalize();
//             Vec3f view = (origin - p).normalize();

//             std::vector<PointLight> _lights;

//             // get lights not shadowing
//             /// using N*1e-3 to make point escape from surface.
//             if (view*N < 0) {
//                 shadowRender(p-N*1e-3, spheres, lights, _lights);
//             } else {
//                 shadowRender(p+N*1e-3, spheres, lights, _lights);
//             }
            
//             if (sphere.material == 0) {
//                 color = PhongRender(view, p, N, sphere.color, _lights);
//             } else if (sphere.material == 1) {
//                 Vec3f reflect_dir = reflect(N, view);
//                 Vec3f reflect_ori = view*N < 0 ? p-N*1e-3 : p+N*1e-3;
//                 // color = ReflectRender(view, p, N, spheres, _lights, 0);
//                 color = RayTracing(reflect_ori, reflect_dir, spheres, lights, depth+1);


//                 float ambient = 0.005f;
//                 float diffuse = 0.9f;
//                 float specular = 0.0f;
//                 for (int i = 0; i < (int)_lights.size(); ++i) {
//                     Vec3f dir = (_lights[i].pos - p).normalize();
//                     // float beta = 1.0f;
//                     // beta = std::max(0.0f, dir*N);
//                     // diffuse += beta*_lights[i].intensity/(_lights[i].pos - p).norm();

//                     // specular
//                     Vec3f h = (view + dir).normalize();
//                     float a = std::max(0.0f, h*N);
//                     specular += std::pow(a, 100.0f)*_lights[i].intensity/(_lights[i].pos - p).norm();
//                 }
//                 color = color * (1.0f * diffuse+ 0.6 * specular) + ambient;
//             } else if (sphere.material == 2) {
//                 float n1 = 1.0f, n2 = 1.5f;
//                 float r_0 = n1/n2;
//                 float c_0 = N * view;
//                 Vec3f refract_v_0 = (N*(r_0*c_0-sqrtf(1-r_0*r_0*(1-c_0*c_0)))-view*r_0).normalize();

//                 float dis_0 = sqrtf(1-r_0*r_0*(1-c_0*c_0))*sphere.radius*2;
//                 Vec3f p_1 = p + refract_v_0*dis_0;
//                 Vec3f N_1 = (sphere.center-p_1).normalize();
//                 // float p_1_2_sc = (p_1-sphere.center).norm()-N_1*1e-5;
//                 p_1 = p_1 - N_1*1e-5;
//                 // if (p_1_2_sc<sphere.radius+1e-5) {
//                 //     p_1 = p_1 - N_1*(sphere.radius-p_1_2_sc+1e-5);
//                 // }
                
//                 float r_1 = 1.0f/r_0;
//                 float c_1 = -N*refract_v_0;
//                 Vec3f refract_v_1 = (N_1*(r_1*c_1-sqrtf(1-r_1*r_1*(1-c_1*c_1)))+refract_v_0*r_1).normalize();

//                 color = RayTracing(p_1, refract_v_1, spheres, lights, depth+1);

//                 float ambient = 0.005f;
//                 float diffuse = 0.9f;
//                 float specular = 0.0f;
//                 for (int i = 0; i < (int)_lights.size(); ++i) {
//                     Vec3f dir = (_lights[i].pos - p).normalize();
//                     // float beta = 1.0f;
//                     // beta = std::max(0.0f, dir*N);
//                     // diffuse += beta*_lights[i].intensity/(_lights[i].pos - p).norm();

//                     // specular
//                     Vec3f h = (view + dir).normalize();
//                     float a = std::max(0.0f, h*N);
//                     specular += std::pow(a, 100.0f)*_lights[i].intensity/(_lights[i].pos - p).norm();
//                 }
//                 color = color * (1.0f * diffuse+ 0.6 * specular) + ambient;
//             }
//         }
//     }
//     return color;
// }

// void RenderRayTraceing() {
//     const int width = 1024;
//     const int height = 762;
//     const float fov = M_PI/2;
//     std::vector<Vec3f> framebuffer(width*height);
//     Vec3f origin(0, 0, 0);
//     Sphere sphere1(Vec3f(-3, 0, -10), 2, Vec3f(0.4, 0.4, 0.3), 1, 0);
//     Sphere sphere2(Vec3f(2, 0, -8), 2, Vec3f(0.8, 0.6, 0.2), 2, 0);
//     Sphere sphere3(Vec3f(-0.7, 0, -6), 1, Vec3f(0.5, 0.2, 1.0), 3, 2);
//     Sphere sphere4(Vec3f(3, 3, -6), 1, Vec3f(0.6, 0.4, 1.0), 4, 1);
//     std::vector<Sphere> spheres;
//     spheres.push_back(sphere1);
//     spheres.push_back(sphere2);
//     spheres.push_back(sphere3);
//     spheres.push_back(sphere4);

//     PointLight pl0(Vec3f(10, 6, 0), 6);
//     PointLight pl1(Vec3f(-5, 6, 0), 6);
//     PointLight pl2(Vec3f(2, 10, 0), 8);
//     std::vector<PointLight> pls;
//     pls.push_back(pl0);
//     pls.push_back(pl1);
//     pls.push_back(pl2);

//     for (int j = 0; j < height; ++j) {
//         for (int i = 0; i < width; ++i) {
//             float x = (2.0f*(i/(float)width)-1.0f)*tan(fov/2)*width/height;
//             float y = -(2.0f*(j/(float)height)-1.0f)*tan(fov/2);     // y 在数据中向下为正
//             Vec3f dir = (Vec3f(x, y, -1)).normalize();
//             framebuffer[i+j*width] = RayTracing(origin, dir, spheres, pls, 0);
//         }
//     }

//     std::string filename = "./out.png";

//     auto data = (unsigned char*)malloc(width*height*3);
//     for (int i = 0; i < width*height; ++i) {
//         for (int j = 0; j < 3; ++j) {
//             data[i*3+j] = (unsigned char)(255*std::max(0.0f, std::min(1.0f, framebuffer[i][j])));
//         }
//     }
    
//     stbi_write_png(filename.c_str(), width, height, 3, data, 0);
// }