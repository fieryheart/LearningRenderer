#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f, tex, norm;
            int itrash, idx, texIdx, normIdx;
            iss >> trash;
            while (iss >> idx >> trash >> texIdx >> trash >> normIdx) {
                idx--; // in wavefront obj all indices start at 1, not zero
                texIdx--;
                normIdx--;
                f.push_back(idx);
                tex.push_back(texIdx);
                norm.push_back(normIdx);
            }
            faces_.push_back(f);
            facesTex_.push_back(tex);
            facesNorm_.push_back(norm);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            texCoord_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            norm_.push_back(v);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << " vt# " << texCoord_.size() << " vn# " << norm_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::ntexCoords() {
    return texCoord_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::faceTex(int idx) {
    return facesTex_[idx];
}

Vec4f Model::vert(int nthface, int nthvert) {
    return Vec4f(verts_[faces_[nthface][nthvert]], 1.f);
}

Vec3f Model::texCoord(int nthface, int nthvert) {
    return texCoord_[facesTex_[nthface][nthvert]];
}

Vec3f Model::norm(int i, int j) {
    return norm_[facesNorm_[i][j]].normalize();
}

TGAColor Model::diffuse(Vec3f uv) {
    return diffusemap_.get(int(uv.x*diffusemap_.get_width()), int(uv.y*diffusemap_.get_height()));
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}