#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../Utils/stb_image.h"

namespace QGL {

ModelType StrangeModel::type() { return MDT_Strange; }
ModelType BuildinModel::type() { return MDT_Buildin; }
ModelType LightModel::type() { return MDT_Light; }

// 2021-05-08: 模型加载
StrangeModel::StrangeModel(const char *filename) : verts(), normals(), textures(), faces(), mtt(MTT_Diffuse) {
    std::cout << "Init Model " << filename << "." << std::endl;
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) {
        std::cout << "Init Model Fail." << std::endl;
        return;
    }
    std::string line;
    while (!in.eof()) {
        // std::cout << "1" << std::endl;
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {    // 点
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) {
                iss >> v[i];
            }
            // v[0] = -v[0];
            // v[1] = -v[1];
            // v[2] = -v[2];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) { // 面
            Vec3i v, vn, vt;
            int vI, vnI, vtI;
            int index = 0;
            iss >> trash;
            while (iss >> vI >> trash >> vtI >> trash >> vnI) {
                vI--, vnI--, vtI--; // in wavefront obj all indices start at 1, not zero
                v[index] = vI, vn[index] = vnI, vt[index]=vtI;
                index++;
            }
            Face f = Face(v, vn, vt);
            faces.push_back(f);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f vt;
            for (int i=0;i<2;i++) iss >> vt[i];
            vt[1] = 1.0f-vt[1]; // the origin is at the left top cornor of image.
            textures.push_back(vt);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f vn;
            for (int i=0;i<3;i++) iss >> vn[i];
            normals.push_back(vn);
        }
    }
    std::cerr << "#v " << verts.size() << std::endl;
    std::cerr << "#vn " << normals.size() << std::endl;
    std::cerr << "#vt " << textures.size() << std::endl;
    std::cerr << "#f " << faces.size() << std::endl;

    rng.seed(std::random_device()());
    distribution = std::uniform_real_distribution<float>(-1, 1);
}

StrangeModel::~StrangeModel() {
    if (diffusemap_) delete diffusemap_;
}

float StrangeModel::brdf(int nthface, Vec3f p, Vec3f wi, Vec3f wo) {
    if (mtt == MT_Diffuse) {
        return 1.0f / (2*M_PI);
    } else {
        return 0.0f;
    }
}

int StrangeModel::nverts() {return (int)verts.size();}
int StrangeModel::nfaces() {return (int)faces.size();}
Vec3f StrangeModel::vert(int nthface, int nthvert) {return verts[faces[nthface].v[nthvert]];}

Vec3f StrangeModel::norm(int nthface, int nthvert) {
    return normals[faces[nthface].vn[nthvert]];
}

Vec3f StrangeModel::norm(int nthface, Vec3f bc) {
    Vec3f vn[3];
    for (int i = 0; i < 3; ++i) vn[i] = norm(nthface, i);
    return vn[0]*bc[0]+vn[1]*bc[1]+vn[2]*bc[2];
}

Vec2f StrangeModel::tex(int nthface, int nthvert) {
    return textures[faces[nthface].vt[nthvert]];
}

// 将节点归一化
void StrangeModel::preprocess() {
    if (verts.empty()) {
        std::cout << "verts of model is empty." << std::endl;
        return;
    }

    Vec3f minV = Vec3f(std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max());
    Vec3f maxV = Vec3f(std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::min());

    for (auto &v : verts) {
        minV[0] = std::min(minV[0], v[0]);
        minV[1] = std::min(minV[1], v[1]);
        minV[2] = std::min(minV[2], v[2]);

        maxV[0] = std::max(maxV[0], v[0]);
        maxV[1] = std::max(maxV[1], v[1]);
        maxV[2] = std::max(maxV[2], v[2]);
    }

    for (int i = 0; i < 3; ++i) {
        minV[i] = std::floor(minV[i]);
        maxV[i] = std::ceil(maxV[i]);
    }

    // std::cout << "minV : " << minV;
    // std::cout << "maxV : " << maxV;

    float l = minV[0], r = maxV[0];
    float b = minV[1], t = maxV[1];
    float n = minV[2], f = maxV[2];

    float scaleN = std::min(l, std::max(b, n));
    float scaleP = std::max(r, std::max(t, f));
    // std::cout << "(scaleN, scaleP) : " << scaleN << ", " << scaleP << std::endl;

    float scale = std::max(std::abs(scaleN), std::abs(scaleP));
    // std::cout << "scale : " << scale << std::endl;

    for (auto &v : verts) {
        v = v / scale;
    }
}

// 加载贴图
void StrangeModel::loadMap(const char *filename, MapType mt) {
    std::cout << "loadMap: ";
    // 加载图片
    int x,y,n;
    unsigned char *raw = stbi_load(filename, &x, &y, &n, 0);
    std::cout << "(" << x << ", " << y << ", " << n << ")" << std::endl;

    Sample2D *sample2D = new Sample2D(raw, x, y, n);
    if (mt == MT_Diffuse) diffusemap_ = sample2D;
    else {
        std::cout << "The map doesn't exist in model." << std::endl;
        delete sample2D;
    }
}

void StrangeModel::sampleDiffuse(Vec2f uv, Vec4f &color) {
    if (diffusemap_) {
        diffusemap_->sample(uv, color);
    } else {
        color = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

void StrangeModel::sampleDiffuse(int nthface, Vec3f bc, Vec4f &color) {
    if (diffusemap_) {
        Vec2f uvs[3], uv;
        for (int i = 0; i < 3; ++i) uvs[i] = tex(nthface, i);
        uv = uvs[0]*bc[0]+uvs[1]*bc[1]+uvs[2]*bc[2];
        sampleDiffuse(uv, color);
    } else {
        color = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

Vec3f StrangeModel::randomRay(int nthface) {
    Vec3i pi = faces[nthface].v;
    float r0 = distribution(rng);
    float r1 = distribution(rng);
    Vec3f x = (verts[pi[1]]-verts[pi[0]]).normalize();
    Vec3f y = (verts[pi[2]]-verts[pi[0]]).normalize();
    return x*r0 + y*r1;
}

void StrangeModel::scale(float width, float height, float depth) {
    Matrix scaleM = Matrix::identity(4);
    scaleM[0][0] = width/2;
    scaleM[1][1] = height/2;
    scaleM[2][2] = depth/2;

    Matrix scaleM_i = scaleM.inverse();

    Vec4f v, vn;
    for (int i = 0; i < verts.size(); ++i) {
        v = Vec4f(verts[i], 1.0f);
        v = scaleM*v;
        verts[i] = v.v3f();
    }
    for (int i = 0; i < normals.size(); ++i) {
        vn = Vec4f(normals[i], 1.0f);
        vn = scaleM_i*vn;
        normals[i] = (vn.v3f()).normalize();
    }
}

void StrangeModel::translate(float x, float y, float z) {
    Matrix trans = Matrix::identity(4);
    trans[0][3] = x;
    trans[1][3] = y;
    trans[2][3] = z;

    Vec4f v;
    for (int i = 0; i < verts.size(); ++i) {
        v = Vec4f(verts[i], 1.0f);
        v = trans*v;
        verts[i] = v.v3f();
    }
}

void StrangeModel::rotate(float x, float y, float z) {
    Matrix rx = Matrix::identity(4);
    Matrix ry = Matrix::identity(4);
    Matrix rz = Matrix::identity(4);

    rx[1][1] = rx[2][2] = cos(x*M_PI/180);
    rx[2][1] = sin(x*M_PI/180);
    rx[1][2] = -rx[2][1];

    ry[0][0] = ry[2][2] = cos(y*M_PI/180);
    ry[0][2] = sin(y*M_PI/180);
    ry[2][0] = -ry[0][2];

    rz[0][0] = rz[1][1] = cos(z*M_PI/180);
    rz[1][0] = sin(z*M_PI/180);
    rz[0][1] = -rz[1][0];

    Matrix r = rx*ry*rz;

    Vec4f v;
    for (int i = 0; i < verts.size(); ++i) {
        v = Vec4f(verts[i], 1.0f);
        v = r*v;
        verts[i] = v.v3f();
    }    
}


//
//
//
BuildinModel::BuildinModel(std::vector<Vec3f> &verts_, 
				 std::vector<Vec3f> &normals_,
				 std::vector<Vec4f> &colors_,
				 std::vector<Vec3i> &faces_,
				 std::vector<MatrialType> &mt_types_) : 
    verts(verts_),
    normals(normals_),
    colors(colors_),
    faces(faces_),
    mt_types(mt_types_) {

    // 设定随机
    rng.seed(std::random_device()());
    distribution = std::uniform_real_distribution<float>(-1, 1);
}

float BuildinModel::brdf(int nthface, Vec3f p, Vec3f wi, Vec3f wo) {
    if (mt_types[nthface] == MT_Diffuse) {
        return 1.0f / (2*M_PI);
    } else {
        return 0.0f;
    }
}

int BuildinModel::nverts() {return verts.size();}
int BuildinModel::nfaces() {return faces.size();}
Vec3f BuildinModel::vert(int nthface, int nthvert) {return verts[faces[nthface][nthvert]];}
Vec3f BuildinModel::norm(int nthface, int nthvert) {
    return normals[nthface];
}

void BuildinModel::sampleDiffuse(int nthface, Vec4f &color) {
    color = colors[nthface];
}

Vec3f BuildinModel::randomRay(int nthface) {
    Vec3i pi = faces[nthface];
    // float r0 = distribution(rng);
    // float r1 = distribution(rng);
    // Vec3f x = verts[pi[1]]-verts[pi[0]]).normalize();
    // Vec3f y = (verts[pi[2]]-verts[pi[0]]).normalize();
    // return x*r0 + y*r1;
    float r0 = distribution(rng);
    float r1 = distribution(rng);
    Vec3f x = (verts[pi[1]]-verts[pi[0]]).normalize();
    Vec3f y = (x^normals[nthface]).normalize();
    return (x*r0+y*r1).normalize();
}

//
//
//
LightModel::LightModel( std::vector<Vec3f> &verts_, 
			   	        std::vector<Vec3f> &normals_,
			            std::vector<Vec3i> &faces_,
				        Vec4f &color_,
				        float intensity_) :
    verts(verts_),
    normals(normals_),
    faces(faces_),
    color(color_),
    intensity(intensity_)
{
    float w = (verts[1]-verts[0]).norm();
    float h = (verts[2]-verts[0]).norm();
    area = w*h;

    // 设定随机
    rng.seed(std::random_device()());
    distribution = std::uniform_real_distribution<float>(0, 1);
}

int LightModel::nverts() {return verts.size();}
int LightModel::nfaces() {return faces.size();}
Vec3f LightModel::vert(int nthface, int nthvert) {return verts[faces[nthface][nthvert]];}
Vec3f LightModel::norm(int nthface, int nthvert) {
    return normals[nthface];
}

Vec3f LightModel::randomSample() {
    float r0 = distribution(rng);
    float r1 = distribution(rng);
    Vec3f randP = verts[0] + (verts[1]-verts[0])*r0;
    randP = randP + (verts[3]-verts[1])*r1;
    return randP;
}

Vec3f LightModel::randomRay(int nthface) {
    Vec3i pi = faces[nthface];
    float r0 = (distribution(rng)-0.5)*2;
    float r1 = (distribution(rng)-0.5)*2;
    Vec3f x = (verts[pi[1]]-verts[pi[0]]).normalize();
    Vec3f y = (verts[pi[2]]-verts[pi[0]]).normalize();
    return x*r0 + y*r1;
}

Vec4f LightModel::getColor() {
    return color;
}

float LightModel::emit() {
    return intensity;
}

float LightModel::pdf() {
    return 1.0f/area;
}

/*
Vec3f Model::randomSampleInP() {
    float r0 = distribution(rng);
    float r1 = distribution(rng);
    Vec3f x = (verts[1]-verts[0]).normalize();
    Vec3f y = (x^normals[0]).normalize();
    return (x*r0+y*r1).normalize();
}
*/
}