#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "object.hpp"
// #define STB_IMAGE_IMPLEMENTATION
// #include "../Utils/stb_image.h"

namespace QGL {

ObjectType StrangeObject::type() { return OBJ_Strange; }
ObjectType BuildinObject::type() { return OBJ_Buildin; }

// 2021-05-08: 模型加载
StrangeObject::StrangeObject(const char *filename) : verts(), normals(), textures(), faces(), mtt(MTT_Diffuse) {
    std::cout << "Init Object " << filename << "." << std::endl;
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) {
        std::cout << "Init Object Fail." << std::endl;
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
}

int StrangeObject::nverts() {return (int)verts.size();}
int StrangeObject::nfaces() {return (int)faces.size();}
Vec3f StrangeObject::vert(int nthface, int nthvert) {return verts[faces[nthface].v[nthvert]];}

Vec3f StrangeObject::norm(int nthface, int nthvert) {
    return normals[faces[nthface].vn[nthvert]];
}

Vec2f StrangeObject::tex(int nthface, int nthvert) {
    return textures[faces[nthface].vt[nthvert]];
}


//
//
//
BuildinObject::BuildinObject(std::vector<Vec3f> &verts_, 
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

int BuildinObject::nverts() {return verts.size();}
int BuildinObject::nfaces() {return faces.size();}
Vec3f BuildinObject::vert(int nthface, int nthvert) {return verts[faces[nthface][nthvert]];}
Vec3f BuildinObject::norm(int nthface, int nthvert) {
    return normals[nthface];
}
}