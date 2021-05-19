#include <cmath>
#include <limits>
#include <cstdlib>
#include <random>
#include <iostream>
#include "qgl.h"

namespace QGL {


int NUM_THREADS = 8;

Matrix MAT_MODEL = Matrix::identity(4);     // 模型空间
Matrix MAT_VIEW = Matrix::identity(4);      // 相机空间
Matrix MAT_PPROJECT = Matrix::identity(4);   // 透视投影空间
Matrix MAT_OPROJECT = Matrix::identity(4);   // 正交投影空间
Matrix MAT_SCREEN = Matrix::identity(4);     // 屏幕空间
Matrix MAT_TRANS = Matrix::identity(4);
Matrix MAT_NORM_TRANS = Matrix::identity(4);
Matrix MAT_NORM_IT = Matrix::identity(4);

Vec4f BACKGROUND_COLOR = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);


int PATH_TRACING_N = 10;
float PATH_TRACING_P_RR = 0.5;
std::mt19937 PATH_TRACING_RNG;
std::uniform_real_distribution<float> PATH_TRACING_UNIFORM_DIST = std::uniform_real_distribution<float>(0, 1);

/*! \brief Convert RGB to HSV color space
  Copy from https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
  Converts a given set of RGB values `r', `g', `b' into HSV
  coordinates. The input RGB values are in the range [0, 1], and the
  output HSV values are in the ranges h = [0, 360], and s, v = [0,
  1], respectively.
  
  \param fR Red component, used as input, range: [0, 1]
  \param fG Green component, used as input, range: [0, 1]
  \param fB Blue component, used as input, range: [0, 1]
  \param fH Hue component, used as output, range: [0, 360]
  \param fS Hue component, used as output, range: [0, 1]
  \param fV Hue component, used as output, range: [0, 1]
  
*/
void RGBtoHSV(float& fR, float& fG, float fB, float& fH, float& fS, float& fV) {
  float fCMax = std::max(std::max(fR, fG), fB);
  float fCMin = std::min(std::min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  
  if(fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH = 360 + fH;
  }
}


/*! \brief Convert HSV to RGB color space
  Copy from https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.
  
  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]
  \param fH Hue component, used as input, range: [0, 360]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]
  
*/
void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
  float fC = fV * fS; // Chroma
  float fHPrime = fmod(fH / 60.0, 6);
  float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
  float fM = fV - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    fR = fC;
    fG = fX;
    fB = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    fR = fX;
    fG = fC;
    fB = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    fR = 0;
    fG = fC;
    fB = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    fR = 0;
    fG = fX;
    fB = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    fR = fX;
    fG = 0;
    fB = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    fR = fC;
    fG = 0;
    fB = fX;
  } else {
    fR = 0;
    fG = 0;
    fB = 0;
  }
  
  fR += fM;
  fG += fM;
  fB += fM;
}


// void SetModelMat() {
//     MAT_MODEL = Matrix::identity(4);
// }

// void SetViewMat(Vec3f eye, Vec3f center, Vec3f up) {
//     Vec3f z = (eye - center).normalize();
//     Vec3f x = (up^z).normalize();
//     Vec3f y = (z^x).normalize();
//     Matrix viewMatrix = Matrix::identity(4);
//     Matrix mat = Matrix::identity(4);
//     for (int i = 0; i < 3; ++i) {
//         viewMatrix[0][i] = x[i];
//         viewMatrix[1][i] = y[i];
//         viewMatrix[2][i] = z[i];

//         mat[i][3] = -center[i];
//     }
//     MAT_VIEW = viewMatrix*mat;
// }

// void LookAt(Vec3f eye, Vec3f center, Vec3f up) {
//     Vec3f z = (eye - center).normalize();
//     Vec3f x = (up^z).normalize();
//     Vec3f y = (z^x).normalize();
//     Matrix viewMatrix = Matrix::identity(4);
//     Matrix mat = Matrix::identity(4);
//     for (int i = 0; i < 3; ++i) {
//         viewMatrix[0][i] = x[i];
//         viewMatrix[1][i] = y[i];
//         viewMatrix[2][i] = z[i];

//         mat[i][3] = -center[i];
//     }
//     MAT_VIEW = viewMatrix*mat;
// }

// void SetPerspectiveProjectMat(Vec3f camera, Vec3f origin) {
//     MAT_PPROJECT = Matrix::identity(4);
//     MAT_PPROJECT[3][2] = -1.f/(camera-origin).norm();
// }

// void SetOrthogonalProjectMat(int l, int r, int b, int t, int n, int f) {
//     MAT_OPROJECT = Matrix::identity(4);
//     Matrix scale = Matrix::identity(4);
//     scale[0][0] = 2.0f/(r-l);
//     scale[1][1] = 2.0f/(t-b);
//     scale[2][2] = 2.0f/(n-f);

//     Matrix traslate = Matrix::identity(4);
//     traslate[0][3] = -(r+l)/2.0f;
//     traslate[1][3] = -(t+b)/2.0f;
//     traslate[2][3] = -(n+f)/2.0f;

//     MAT_OPROJECT = scale*traslate*MAT_OPROJECT;
// }

// void SetOrthogonalProjectMat(int width, int height, int depth) {
//     MAT_OPROJECT = Matrix::identity(4);
//     Matrix scale = Matrix::identity(4);
//     scale[0][0] = 2.0f/width;
//     scale[1][1] = 2.0f/height;
//     scale[2][2] = 2.0f/depth;

//     Matrix traslate = Matrix::identity(4);

//     MAT_OPROJECT = scale*traslate*MAT_OPROJECT;
// }

// // 计算屏幕坐标
// // x: [-1,1] -> [x,x+w]
// // y: [-1,1] -> [y,y+h]
// // z: [-1,1] -> [0,depth]
// void SetScreenMat(int x, int y, int w, int h, int depth) {
//     MAT_SCREEN = Matrix::identity(4);
//     MAT_SCREEN[0][0] = w/2.f;
//     MAT_SCREEN[1][1] = h/2.f;
//     MAT_SCREEN[2][2] = depth/2.f;

//     MAT_SCREEN[0][3] = x+w/2.f;
//     MAT_SCREEN[1][3] = y+h/2.f;
//     MAT_SCREEN[2][3] = depth/2.f;
// }

// void SetCamera(bool isVNormalized) {
//     if (isVNormalized) {    
//         // 模型的顶点数据已经归一化
//         MAT_TRANS = MAT_SCREEN*MAT_PPROJECT*MAT_VIEW*MAT_MODEL;
//     } else {
//         // 这里针对没有归一化的顶点数据还存在问题
//         MAT_TRANS = MAT_SCREEN*MAT_OPROJECT*MAT_PPROJECT*MAT_VIEW*MAT_MODEL;
//     }
//     MAT_NORM_TRANS = MAT_MODEL.inverse().transpose();
//     // MAT_NORM_IT = MAT_NORM_TRANS.inverse().transpose();
// }

// Vec3f barycentric(Vec4f *pts, Vec2f P) {
//     Vec3f u = (Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x))^(Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
//     if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
//     return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
// }

// void Rendering(RenderNode &rn) {
//     if (rn.comType == CT_Single) SingleRendering(rn);
//     else if (rn.comType == CT_Omp) OmpRendering(rn);
//     else return;
// }

// void SingleRendering(RenderNode &rn) {
//     Model *model = rn.model;
//     Shader *shader = rn.shader;
//     Log *log = rn.log;
//     // Zbuffer *zbuffer = rn.zbuffer;
// 	Frame *frame = rn.frame;
//     Vec4f screen_coords[3];
//     int nfaces = model->nfaces();
//     for (int i = 0; i < nfaces; ++i) {
//         for (int j = 0; j < 3; ++j) {
//             Vec3f v = model->vert(i, j);
//             InVectex inV;
//             OutVectex outV;

//             inV.v = v;
//             inV.nthface = i;
//             inV.nthvert = j;
//             inV.model = model;
            
//             shader->vertex(inV, outV);
//             screen_coords[j] = outV.sCoord;
//             // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
//         }

//         // float zA = zbuffer->get(int(screen_coords[0][0]), int(screen_coords[0][1]));
//         // float zB = zbuffer->get(int(screen_coords[1][0]), int(screen_coords[1][1]));
//         // float zC = zbuffer->get(int(screen_coords[2][0]), int(screen_coords[2][1]));
//         // if (zA >= screen_coords[0][2] && zB >= screen_coords[1][2] && zC >= screen_coords[2][2]) {
//         //     continue;
//         // }

//         DrawTriangle(screen_coords, rn);

//         if (log && log->flag) log->show(i, nfaces);
//     }
//     if (log && log->flag) log->show(nfaces, nfaces);
// }

// // Omp光栅化未处理好
// void OmpRendering(RenderNode &rn) {
//     Model *model = rn.model;
//     Shader *shader = rn.shader;
//     Log *log = rn.log;
//     // Zbuffer *zbuffer = rn.zbuffer;

//     Vec4f screen_coords[3];
//     int nfaces = model->nfaces();

//     #pragma omp parallel for num_threads(NUM_THREADS)
//     for (int i = 0; i < nfaces; ++i) {
//         for (int j = 0; j < 3; ++j) {
//             Vec3f v = model->vert(i, j);
//             InVectex inV;
//             OutVectex outV;

//             inV.v = v;
//             inV.nthface = i;
//             inV.nthvert = j;
//             inV.model = model;
            
//             shader->vertex(inV, outV);
//             screen_coords[j] = outV.sCoord;
//             // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
//         }

//         // float zA = zbuffer->get(int(screen_coords[0][0]), int(screen_coords[0][1]));
//         // float zB = zbuffer->get(int(screen_coords[1][0]), int(screen_coords[1][1]));
//         // float zC = zbuffer->get(int(screen_coords[2][0]), int(screen_coords[2][1]));
//         // if (zA >= screen_coords[0][2] && zB >= screen_coords[1][2] && zC >= screen_coords[2][2]) {
//         //     continue;
//         // }

//         DrawTriangle(screen_coords, rn);

//         // if (log && log->flag) log->show(i, nfaces);
//     }
//     // if (log && log->flag) log->show(nfaces, nfaces);
// }

// void DrawTriangle(Vec4f *points, RenderNode &rn) {
//     Model *model = rn.model;
//     Shader *shader = rn.shader;
//     Frame *frame = rn.frame;
//     Zbuffer *zbuffer = rn.zbuffer;
//     int width = frame->width, height = frame->height;

//     Vec2f bboxmin(width-1, height-1);
//     Vec2f bboxmax(0, 0);
//     Vec2f clamp(width-1, height-1); 
//     for (int i=0; i<3; i++) { 
//         bboxmin.x = std::min(bboxmin.x, points[i].x);
//         bboxmin.y = std::min(bboxmin.y, points[i].y);
//         bboxmax.x = std::max(bboxmax.x, points[i].x);
//         bboxmax.y = std::max(bboxmax.y, points[i].y);
//     }
//     bboxmin.x = std::min(0.f, bboxmin.x);
//     bboxmin.y = std::min(0.f, bboxmin.y);
//     bboxmax.x = std::max(clamp.x, bboxmax.x);
//     bboxmax.y = std::max(clamp.y, bboxmax.y);

//     Vec2i P;
//     Vec3f bc;
//     float z, w, depth, weight;
//     // float r,g,b,h,s,v;
//     for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
//         for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
//             bc = barycentric(points, Vec2f(P.x, P.y));
//             z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
//             w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
//             depth = std::max(0.f, std::min(1.0f, z/w));
//             if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer->get(P.x, P.y) > depth) continue;
//             // if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zb[P.x] > depth) continue;
//             // if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
//             InFragment in;
//             OutFragment out;
//             in.bar = bc;
//             in.depth = z;
//             in.model = model;
//             if (!shader->fragment(in, out)) {
//                 zbuffer->set(P.x, P.y, z);
//                 frame->set(P.x, P.y, out.color);
//             }
//             // if (!shader->fragment(in, out)) {
//             //     zb[P.x] = depth;
//             //     frame->set(P.x, P.y, out.color);
//             // }
//         }
//     }
//     // int dw = bboxmax.x-bboxmin.x, dh = bboxmax.y-bboxmin.y;
//     // for (int i = 0; i < dw*dh; ++i) {
//     //     P.x = bboxmin.x + i % dw;
//     //     P.y = bboxmin.y + i / dw;
//     //     Vec3f bc = barycentric(points, Vec2f(P.x, P.y));
//     //     float z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
//     //     float w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
//     //     float depth = std::max(0.f, std::min(1.0f, z/w));        
//     //     if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
//     //     zbuffer[P.x+width*P.y] = depth;
//     // }
// }

// ? 可以改成随机取
void chooseN(int i, int j, float &x, float &y, int k) {
    float cx = i + 0.5f, cy = j + 0.5f;
    float r = 2*M_PI/PATH_TRACING_N*k;
    x = cx + 0.5*cos(r);
    y = cy + 0.5*sin(r);
}


bool RayInteract(BVHBuilder *bvh, Ray &ray, int &index, Vec3f &bc, Vec3f &p) {
    std::vector<int> indices;
    bvh->interact(ray, indices);
    // get the nearest triangle which ray reaches.
    InInteract in;
    OutInteract out;
    in.ray = ray;
    in.t = std::numeric_limits<float>::max();
    out.idx = -1;
    for (int i = 0; i < indices.size(); ++i) {
        in.idx = indices[i];
        bvh->tris[indices[i]]->interact(in, out);
    }
    if (out.idx == -1) {
        index = -1;
        return false;
    } else {
        index = out.idx;
        bc = out.bc;
        p = ray.launch(out.t);

        // std::cout << "tri:" << bvh->tris[index]->vert[0];
        // std::cout << "tri:" << bvh->tris[index]->vert[1];
        // std::cout << "tri:" << bvh->tris[index]->vert[2];
        // std::cout << "RayInteract: " << out.t << std::endl;
        return true;
    }
}

// 路径追踪
void RenderingByPathTracing(RenderPTNode &in) {

    // 初始化随机种子
    std::mt19937 PATH_TRACING_RNG;
    PATH_TRACING_RNG.seed(std::random_device()());

    BVHBuilder *bvh = new BVHBuilder(in.models);

    std::cout << bvh->tris.size() << std::endl;

    Frame *frame = in.frame;
    Vec3f pos = in.camera;
    int width = in.width, height = in.height;
    float fov = in.fov;
    int bound = in.bound;

    // Test: one ray
    // float x, y;
    // Vec3f dir;
    // Ray ray;
    // ray.pos = pos;
    // Vec4f color = Vec4f(0.0f);  
    // Vec4f one_ray_color = Vec4f(0.0f);
    // int index;
    // Vec3f bc;
    // Vec3f p;
    // for (int i = 380; i <= 386; ++i) {
    //     std::cout << i << std::endl;
    //     for (int k = 0; k < PATH_TRACING_N; ++k) {
    //         std::cout << "k: " << k << std::endl;

    //         chooseN(i,57,x,y,k);
    //         x = (2.0f*(x/(float)width)-1.0f)*tan(fov/2)*width/height;
    //         y = -(2.0f*(y/(float)height)-1.0f)*tan(fov/2);  // y 在数据中向下为正
    //         dir = (Vec3f(x, y, -1)).normalize();
    //         ray.dir = dir;

    //         if (RayInteract(bvh, ray, index, bc, p)) {
    //             one_ray_color = RayTracing(bvh, ray, index, bc, p);
    //             one_ray_color[0] = std::min(1.0f, one_ray_color[0]);
    //             one_ray_color[1] = std::min(1.0f, one_ray_color[1]);
    //             one_ray_color[2] = std::min(1.0f, one_ray_color[2]);
    //         } else {
    //             one_ray_color = Vec4f(0.0f);
    //         }

    //         color = color + one_ray_color / (PATH_TRACING_N*1.0f);
    //     }
    //     color[3] = 1.0f;
    //     std::cout << "color: " << color;
    // }


    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            float x, y;
            Vec3f dir;
            Ray ray;
            ray.pos = pos;
            Vec4f color = Vec4f(0.0f);
            Vec4f one_ray_color = Vec4f(0.0f);

            int index;
            Vec3f bc;
            Vec3f p;

            // uniformly choose N sample positions within the pixel
            for (int k = 0; k < PATH_TRACING_N; ++k) {
                chooseN(i,j,x,y,k);

                x = (2.0f*(x/(float)width)-1.0f)*tan(fov/2)*width/height;
                y = -(2.0f*(y/(float)height)-1.0f)*tan(fov/2);  // y 在数据中向下为正
                dir = (Vec3f(x, y, -1)).normalize();
                ray.dir = dir;

                if (RayInteract(bvh, ray, index, bc, p)) {
                    one_ray_color = RayTracing(bvh, ray, index, bc, p);
                    one_ray_color[0] = std::min(1.0f, one_ray_color[0]);
                    one_ray_color[1] = std::min(1.0f, one_ray_color[1]);
                    one_ray_color[2] = std::min(1.0f, one_ray_color[2]);
                } else {
                    one_ray_color = Vec4f(0.0f);
                }

                color = color + one_ray_color / (PATH_TRACING_N*1.0f);
            }
            // std::cout << dir;
            color[3] = 1.0f;
            frame->set(i, j, color);
        }
    }
}


//
// 参数：
//      bvh, index, p, bc
//
Vec4f RayTracing(BVHBuilder *bvh, Ray &ray, int index, Vec3f bc, Vec3f p) {
    Vec3f lp, next_p, N_p, N_lp;
    BVHTriangle *tri = bvh->tris[index];
    float k_r = 0.0f, f_r = 0.0f, L_i = 0.0f, dis = 0.0f;
    Vec4f lcolor, diffColor, L_e = Vec4f(0.0f), L_dir = Vec4f(0.0f), L_indir = Vec4f(0.0f);
    Vec3f w0, w1;

    /*
    * 
    *  Contribution from itself.
    *
    */
    if (tri->model->type() == MDT_Light) {
        LightModel *lm = dynamic_cast<LightModel*>(tri->model);
        lcolor = lm->getColor();
        L_i = lm->emit();
        f_r = 1 / (2*M_PI);
        k_r = L_i*f_r/(ray.pos-p).norm()/lm->pdf();
        L_e = lcolor*k_r;
    }
    // return L_e;

    /*
    * 
    *  Contribution from the light source
    *
    */
    L_i = bvh->light->emit();
    if (tri->model->type() == MDT_Buildin) {
        BuildinModel *bm = dynamic_cast<BuildinModel*>(tri->model);
        N_p = bm->norm(tri->nthface, 0);
        p = p+N_p*0.5f;
        lp = bvh->light->randomSample(); // uniformly sample the light.
        N_lp = bvh->light->norm(0, 0);
        w0 = (ray.pos-p).normalize();
        w1 = (p-lp).normalize();

        // occlusion
        Ray trashRay = Ray(p, -w1);
        int tmpIdx = -1;
        Vec3f trashBC, tmpP;
        RayInteract(bvh, trashRay, tmpIdx, trashBC, tmpP);
        // 只有一个光源，暂时先这么写～
        if (tmpIdx != -1 && bvh->tris[tmpIdx]->model->type() == MDT_Light) {
            dis = (p-lp).norm2();
            f_r = bm->brdf(tri->nthface, p, w0, -w1);
            k_r = L_i * f_r * (N_p*w0) * (N_lp*w1) / dis / bvh->light->pdf();
            bm->sampleDiffuse(tri->nthface, diffColor);
            L_dir = diffColor*k_r;
        }
    } else if (tri->model->type() == MDT_Strange) {

    }
    // return L_e + L_dir;

    /*
    * 
    *  Contribution from other reflections.
    *
    */
    float ksi = PATH_TRACING_UNIFORM_DIST(PATH_TRACING_RNG);
    if (ksi > PATH_TRACING_P_RR) return L_e + L_dir;

    // Randomly choose ONE direction wi~pdf(w)
    // ?    方向选定不够随机
    Vec3f randp = tri->model->randomRay(tri->nthface);
    Vec3f dir = (randp + N_p).normalize();
    Ray next_ray = Ray(p, dir);

    index = -1;
    RayInteract(bvh, next_ray, index, bc, next_p);
    if (index == -1 || bvh->tris[index]->model->type() == MDT_Light) return L_e + L_dir;
    L_indir = RayTracing(bvh, next_ray, index, bc, next_p) * f_r * (N_p*w0) * 2 * M_PI / PATH_TRACING_P_RR;

    return L_e + L_dir + L_indir;
}
}

// /* 
//  *  作用：根据 点A 和 点B 画线。
//  *  参数：
//  *      x0: 点A的x值
//  *      y0: 点A的y值
//  *      x1: 点B的x值
//  *      y1: 点B的y值
//  *      image: TGA图片对象
//  *      color: TGA颜色
//  *  返回：无
//  */
// void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
//     int x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
//     bool steep = false; 
//     if (std::abs(x0-x1)<std::abs(y0-y1)) { 
//         std::swap(x0, y0); 
//         std::swap(x1, y1); 
//         steep = true; 
//     } 
//     if (x0>x1) { 
//         std::swap(x0, x1); 
//         std::swap(y0, y1); 
//     } 
//     int dx = x1-x0; 
//     int dy = y1-y0; 
//     int derror2 = std::abs(dy)*2; 
//     int error2 = 0; 
//     int y = y0; 
//     for (int x=x0; x<=x1; x++) { 
//         if (steep) {
//             image.set(y, x, color); 
//         } else { 
//             image.set(x, y, color); 
//         } 
//         error2 += derror2; 
//         if (error2 > dx) { 
//             y += (y1>y0?1:-1); 
//             error2 -= dx*2; 
//         } 
//     }     
// }