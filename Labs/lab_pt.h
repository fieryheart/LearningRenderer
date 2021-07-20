#ifndef __LAB_PT_H__
#define __LAB_PT_H__

#include "../RayTracing/PathTracing.h"

using namespace QGL;

void Lab_PT() {
    const int width = 800;
    const int height = 800;
    Vec3f camera = Vec3f(0.0f, 0.0f, 0.0f);
    Frame frame = Frame(width, height, 4);

    // Test BVH
    PTNode rn;
    rn.frame = &frame;
    rn.camera = camera;
    rn.width = width;
    rn.height = height;
    rn.fov = M_PI*0.7;
    rn.bound = 1;

    // Plane Model
    // 中
    // Vec3f pts0[4] = {Vec3f(-50, -60, -100), Vec3f(-50, 60, -100),
    //                 Vec3f(50, -60, -100),  Vec3f(50, 60, -100)};
    // Vec4f color0 = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    // QGL::Plane plane0 = QGL::Plane(pts0, color0);
    // plane0.emission = 0.0f;
    // QGL::Model *planeModel0 = new QGL::Model(plane0);

    // // 左
    // Vec3f pts1[4] = {Vec3f(-60, -60, -50), Vec3f(-60, 60, -50), 
    //                  Vec3f(-50, -60, -100), Vec3f(-50, 60, -100)};
    // Vec4f color1 = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    // QGL::Plane plane1 = QGL::Plane(pts1, color1);
    // plane1.emission = 0.0f;
    // QGL::Model *planeModel1 = new QGL::Model(plane1);

    // // 上
    // Vec3f pts2[4] = {Vec3f(-50, 60, -100), Vec3f(-60, 60, -50),
    //                 Vec3f(50, 60, -100),  Vec3f(60, 60, -50)};
    // Vec4f color2 = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    // QGL::Plane plane2 = QGL::Plane(pts2, color2);
    // plane2.emission = 0.0f;
    // QGL::Model *planeModel2 = new QGL::Model(plane2);

    // // 右
    // Vec3f pts3[4] = {Vec3f(50, -60, -100), Vec3f(50, 60, -100),
    //                  Vec3f(60, -60, -50),  Vec3f(60, 60, -50)};
    // Vec4f color3 = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
    // QGL::Plane plane3 = QGL::Plane(pts3, color3);
    // plane3.emission = 0.0f;
    // QGL::Model *planeModel3 = new QGL::Model(plane3);

    // // 下
    // Vec3f pts4[4] = {Vec3f(-60, -60, -50), Vec3f(-50, -60, -100),
    //                  Vec3f(60, -60, -50),  Vec3f(50, -60, -100)};
    // Vec4f color4 = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    // QGL::Plane plane4 = QGL::Plane(pts4, color4);
    // plane4.emission = 0.0f;
    // QGL::Model *planeModel4 = new QGL::Model(plane4);

    // // light
    // Vec3f pts5[4] = {Vec3f(-10, 59.9, -80), Vec3f(-10, 59.9, -70),
    //                  Vec3f(10, 59.9, -80),  Vec3f(10, 59.9, -70)};
    // Vec4f color5 = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    // QGL::Plane plane5 = QGL::Plane(pts5, color5);
    // plane5.emission = 150.0f;
    // QGL::Model *planeModel5 = new QGL::Model(plane5);

    // Marry
    // QGL::StrangeModel *marry = new QGL::StrangeModel("../asset/Marry/Marry.obj");
    // marry->preprocess();
    // marry->scale(80, 80, 100);
    // marry->translate(0, -20, -25);
    // marry->loadMap("../obj/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);

    // Planes
    float l = -50, r = 50;
    float b = -50, t = 50;
    float f = -49, n = 1;
    std::vector<Vec3f> planes_verts{Vec3f(l, t, n), Vec3f(l, b, n),
                                    Vec3f(l, b, f), Vec3f(l, t, f),
                                    Vec3f(r, t, f), Vec3f(r, b, f),
                                    Vec3f(r, b, n), Vec3f(r, t, n)};
    std::vector<Vec3f> normals_verts{Vec3f(1, 0, 0), Vec3f(1, 0, 0),
                                     Vec3f(0, -1, 0), Vec3f(0, -1, 0),
                                     Vec3f(-1, 0, 0), Vec3f(-1, 0, 0),
                                     Vec3f(0, 1, 0), Vec3f(0, 1, 0),
                                     Vec3f(0, 0, 1), Vec3f(0, 0, 1),
                                     Vec3f(0, 0, -1), Vec3f(0, 0, -1)};
    std::vector<Vec4f> colors{Vec4f(1.0f, 0.0f, 0.0f, 1.0f),Vec4f(1.0f, 0.0f, 0.0f, 1.0f),
                              Vec4f(1.0f, 1.0f, 1.0f, 1.0f),Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                              Vec4f(0.0f, 1.0f, 0.0f, 1.0f),Vec4f(0.0f, 1.0f, 0.0f, 1.0f),
                              Vec4f(1.0f, 1.0f, 1.0f, 1.0f),Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                              Vec4f(1.0f, 1.0f, 1.0f, 1.0f),Vec4f(1.0f, 1.0f, 1.0f, 1.0f),
                              Vec4f(1.0f, 1.0f, 1.0f, 1.0f),Vec4f(1.0f, 1.0f, 1.0f, 1.0f)};
    std::vector<Vec3i> faces{Vec3f(0, 1, 2), Vec3f(0, 2, 3),    // 左
                             Vec3f(0, 3, 4), Vec3f(0, 4, 7),    // 上
                             Vec3f(4, 5, 6), Vec3f(4, 6, 7),    // 右
                             Vec3f(1, 2, 5), Vec3f(1, 5, 6),    // 下
                             Vec3f(2, 3, 5), Vec3f(3, 4, 5),    // 后
                             Vec3f(0, 1, 6), Vec3f(0, 6, 7)     // 前
                            };
    std::vector<QGL::MatrialType> mt_types{QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse};
    QGL::Model *box = new QGL::BuildinModel(planes_verts, normals_verts, colors, faces, mt_types);



    // light
    std::vector<Vec3f> light_verts{Vec3f(-10, t-0.1, -30), Vec3f(-10, t-0.1, -20),
                                   Vec3f(10, t-0.1, -30),  Vec3f(10, t-0.1, -20)};
    std::vector<Vec3f> light_normals{Vec3f(0, -1, 0), Vec3f(0, -1, 0)};
    std::vector<Vec3i> light_faces{Vec3f(0, 1, 2), Vec3f(1, 2, 3)};
    Vec4f light_color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    float intensity = 150.0f;
    QGL::Model *light = new QGL::LightModel(light_verts, light_normals, light_faces, 
                                            light_color, intensity);
    
    std::vector<QGL::Model*> models;
    // models.push_back(marry);
    models.push_back(box);
    models.push_back(light);

    // models.push_back(planeModel0);
    // models.push_back(planeModel1);
    // models.push_back(planeModel2);
    // models.push_back(planeModel3);
    // models.push_back(planeModel4);
    // models.push_back(planeModel5);

    // std::cout<< "plane0.normal: " << plane0.normal << std::endl;
    // std::cout<< "plane1.normal: " << plane1.normal << std::endl;
    // std::cout<< "plane2.normal: " << plane2.normal << std::endl;
    // std::cout<< "plane3.normal: " << plane3.normal << std::endl;
    // std::cout<< "plane4.normal: " << plane4.normal << std::endl;
    // std::cout<< "plane5.normal: " << plane5.normal << std::endl;

    rn.models = models;
    // rn.light = 5;

    QGL::Timer timer = QGL::Timer();
    timer.update();
    QGL::RenderingByPathTracing(rn);
    timer.show();

    std::cout << "draw frame." << std::endl;
    std::string out = "../examples/PathTracing/box.png";
    // frame.flip();
    frame.draw(out.c_str());

    // Test AxisAlignedBoundBox
    // QGL::AxisAlignedBoundBox aabb = QGL::AxisAlignedBoundBox(1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 2.0f);
    // Vec3f pos = Vec3f(0.0f, 0.0f, 0.0f);
    // Vec3f dir = (Vec3f(1.0f, 1.0f, 1.5f)).normalize();
    // QGL::Ray ray = QGL::Ray(pos, dir);
    // bool isin = aabb.interact(ray);
    // std::cout << "Interact: " << isin << std::endl;
}


#endif