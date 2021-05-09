#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace QGL {
// 2021-05-08: 模型加载
Model::Model(const char *filename) : verts(), normals(), textures(), faces() {
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
            for (int i=0;i<3;i++) iss >> v[i];
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
    // load_texture(filename, "_diffuse.tga", diffusemap_);
    // load_texture(filename, "_nm.tga",      normalmap_);
    // load_texture(filename, "_spec.tga",    specularmap_);
}

Model::Model(std::vector<Vec3f> &_verts, std::vector<Face> _faces) {
    verts = _verts;
    faces = _faces;
}

Model::~Model() {
    if (diffusemap_) delete diffusemap_;
}

int Model::nverts() {
    return (int)verts.size();
}

int Model::nfaces() {
    return (int)faces.size();
}

Vec3f Model::vert(int nthface, int nthvert) {
    return verts[faces[nthface].v[nthvert]];
}

Vec2f Model::tex(int nthface, int nthvert) {
    return textures[faces[nthface].vt[nthvert]];
}

// 将节点归一化
void Model::vertsNormalize() {
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
void Model::loadMap(const char *filename, MapType mt) {
    std::cout << "loadMap" << std::endl;
    // 加载图片
    int x,y,n;
    unsigned char *raw = stbi_load(filename, &x, &y, &n, 0);
    std::cout << "width: " << x << std::endl;
    std::cout << "height: " << y << std::endl;
    std::cout << "channel: " << n << std::endl;

    Sample2D *sample2D = new Sample2D(raw, x, y, n);
    if (mt == MT_Diffuse) diffusemap_ = sample2D;
    else {
        std::cout << "The map doesn't exist in model." << std::endl;
        delete sample2D;
    }
}

void Model::sampleDiffuse(Vec2f uv, Vec4f &color) {
    diffusemap_->sample(uv, color);
}
}


// int Model::nverts() {
//     return (int)verts_.size();
// }

// int Model::ntexCoords() {
//     return texCoord_.size();
// }

// int Model::nfaces() {
//     return (int)faces_.size();
// }

// std::vector<int> Model::face(int idx) {
//     return faces_[idx];
// }

// std::vector<int> Model::faceTex(int idx) {
//     return facesTex_[idx];
// }

// Vec4f Model::vert(int nthface, int nthvert) {
//     return Vec4f(verts_[faces_[nthface][nthvert]], 1.f);
// }

// Vec3f Model::texCoord(int nthface, int nthvert) {
//     return texCoord_[facesTex_[nthface][nthvert]];
// }

// Vec3f Model::norm(int i, int j) {
//     return norm_[facesNorm_[i][j]].normalize();
// }

// Vec3f Model::normal(Vec3f uv) {
//     TGAColor c = normalmap_.get(int(uv.x*diffusemap_.get_width()), int(uv.y*diffusemap_.get_height()));
//     Vec3f res;
//     for (int i=0; i<3; i++)
//         res[2-i] = (float)c[i]/255.f*2.f - 1.f;
//     return res;
// }

// TGAColor Model::diffuse(Vec3f uv) {
//     return diffusemap_.get(int(uv.x*diffusemap_.get_width()), int(uv.y*diffusemap_.get_height()));
// }

// float Model::specular(Vec3f uv) {
//     return specularmap_.get(int(uv[0]*specularmap_.get_width()), int(uv[1]*specularmap_.get_height()))[0]/1.f;
// }

// void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
//     std::string texfile(filename);
//     size_t dot = texfile.find_last_of(".");
//     if (dot!=std::string::npos) {
//         texfile = texfile.substr(0,dot) + std::string(suffix);
//         std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
//         img.flip_vertically();
//     }
// }

// int Model::getDiffuseMapWidth() {
//     return diffusemap_.get_width();
// }

// int Model::getDiffuseMapHeight() {
//     return diffusemap_.get_height();
// }

// Vec3f barycentric(Vec3f *pts, Vec2f P) {
//     Vec3f u = (Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x))^(Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
//     if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
//     return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
// }

// not to generate normal mapping by using interpolation
// void Model::generate_normal_mapping(const char *filename) {
//     TGAImage image(diffusemap_.get_width(), diffusemap_.get_height(), TGAImage::RGB);
//     for (int i=0; i<nfaces(); i++) {
//         Vec3f uv[3];
//         Vec3f normals[3];
//         for (int j=0; j<3; j++) {
//             uv[j] = texCoord(i, j);
//             uv[j].x = uv[j].x * diffusemap_.get_width();
//             uv[j].y = uv[j].y * diffusemap_.get_height();
//             normals[j] = norm(i, j);
//         }
//         // triangle(screen_coords, phongShader, image, zbuffer);

//         Vec2f bboxmin(image.get_width()-1,  image.get_height()-1); 
//         Vec2f bboxmax(0, 0); 
//         Vec2f clamp(image.get_width()-1, image.get_height()-1); 
//         for (int i=0; i<3; i++) {
//             bboxmin.x = std::min(bboxmin.x, uv[i].x);
//             bboxmin.y = std::min(bboxmin.y, uv[i].y);
//             bboxmax.x = std::max(bboxmax.x, uv[i].x);
//             bboxmax.y = std::max(bboxmax.y, uv[i].y);
//         }
//         bboxmin.x = std::min(0.f, bboxmin.x);
//         bboxmin.y = std::min(0.f, bboxmin.y);
//         bboxmax.x = std::max(clamp.x, bboxmax.x);
//         bboxmax.y = std::max(clamp.y, bboxmax.y);
        
//         // std::cout << uv[0] << std::endl;
//         // std::cout << uv[1] << std::endl;
//         // std::cout << uv[2] << std::endl;

//         Vec2i P;
//         TGAColor color;
//         long lR, lG, lB;
//         unsigned char R, G, B;
//         for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
//             for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
//                 Vec3f bc = barycentric(uv, Vec2f(P.x, P.y));
//                 Vec3f n = normals[0]*bc.x+normals[1]*bc.y+normals[2]*bc.z;
//                 // std::cout << bc << std::endl;
//                 if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
//                 n = (n*0.5f+0.5f) * 255.0f;
//                 // std::cout << n << std::endl;
//                 lR = (long)n.x;
//                 lG = (long)n.y;
//                 lB = (long)n.z;
//                 R = (unsigned char)lR;
//                 G = (unsigned char)lG;
//                 B = (unsigned char)lB;
//                 color = TGAColor(R,G,B);
//                 image.set(P.x, P.y, color);
//             }
//         }
//     }

//     image.flip_vertically();
//     image.write_tga_file(filename);
// }