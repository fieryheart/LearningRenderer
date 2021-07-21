#ifndef __LAB_PT_H__
#define __LAB_PT_H__

#include "../RayTracing/PathTracing.h"
#define MY_EPS 1e-5
using namespace QGL;

void Lab_PT() {
    const int width = 2400;
    const int height = 2400;
    Vec3f camera = Vec3f(0.0f, 0.0f, 0.0f);
    Frame frame = Frame(width, height, 4);

    // Planes
    float l = -10, r = 10;
    float b = -8, t = 8;
    float f = -21, n = -11;
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
                            //  Vec3f(0, 1, 6), Vec3f(0, 6, 7)     // 前
                            };
    std::vector<QGL::MatrialType> mt_types{QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Diffuse,QGL::MTT_Diffuse,
                                           QGL::MTT_Transparent,QGL::MTT_Transparent};
    QGL::Model *box = new QGL::BuildinModel(planes_verts, normals_verts, colors, faces, mt_types);



    // light
    std::vector<Vec3f> light_verts{Vec3f(l/5, t, n), Vec3f(l/5, t-3, n+1),
                                   Vec3f(r/5, t, n),  Vec3f(r/5, t-3, n+1)};
    Vec3f light_dir = (Vec3f(0, -0.5, -1)).normalize();
    std::vector<Vec3f> light_normals{light_dir, light_dir};
    std::vector<Vec3i> light_faces{Vec3f(0, 1, 2), Vec3f(1, 2, 3)};
    Vec4f light_color = Vec4f(255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
    // std::cout << light_color;
    float intensity = 300.0f;
    QGL::Model *light = new QGL::LightModel(light_verts, light_normals, light_faces, 
                                            light_color, intensity);
    

    // Marry
    QGL::StrangeModel *marry = new QGL::StrangeModel("../asset/Marry/Marry.obj");
    marry->scale(6, 6, 6);
    marry->rotate(0, 0, 0);
    marry->translate(0, b, (n+f)/2+1);
    marry->loadMap("../asset/Marry/MC003_Kozakura_Mari.png", QGL::MT_Diffuse);


    std::vector<QGL::Model*> models;
    models.push_back(marry);
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

    // Test BVH
    PTNode ptn;
    ptn.frame = &frame;
    ptn.camera = camera;
    ptn.width = width;
    ptn.height = height;
    ptn.fov = M_PI*0.5;
    ptn.bound = 1;
    ptn.models = models;
    // ptn.light = 5;

    QGL::Timer timer = QGL::Timer();
    timer.update();
    PathTracing(ptn);
    timer.show();

    std::cout << "draw frame." << std::endl;
    std::string out = "../examples/PathTracing/box.png";
    // frame.flip();
    frame.draw(out.c_str());
}


#endif