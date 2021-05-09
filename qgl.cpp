#include <cmath>
#include <limits>
#include <cstdlib>
#include <iostream>
#include "qgl.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace QGL {
Matrix MAT_MODEL;   // 模型空间
Matrix MAT_VIEW;    // 相机空间
Matrix MAT_PPROJECT; // 透视投影空间
Matrix MAT_OPROJECT; // 正交投影空间
Matrix MAT_SCREEN;  // 屏幕空间
Matrix MAT_TRANS;

void SetModelMat() {
    MAT_MODEL = Matrix::identity(4);
}

void SetViewMat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    Matrix viewMatrix = Matrix::identity(4);
    Matrix mat = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        viewMatrix[0][i] = x[i];
        viewMatrix[1][i] = y[i];
        viewMatrix[2][i] = z[i];

        mat[i][3] = -center[i];
    }
    MAT_VIEW = viewMatrix*mat;
}

void LookAt(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    Matrix viewMatrix = Matrix::identity(4);
    Matrix mat = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        viewMatrix[0][i] = x[i];
        viewMatrix[1][i] = y[i];
        viewMatrix[2][i] = z[i];

        mat[i][3] = -center[i];
    }
    MAT_VIEW = viewMatrix*mat;
}

void SetPerspectiveProjectMat(Vec3f camera, Vec3f origin) {
    MAT_PPROJECT = Matrix::identity(4);
    MAT_PPROJECT[3][2] = -1.f/(camera-origin).norm();
}

void SetOrthogonalProjectMat(int l, int r, int b, int t, int n, int f) {
    MAT_OPROJECT = Matrix::identity(4);
    Matrix scale = Matrix::identity(4);
    scale[0][0] = 2.0f/(r-l);
    scale[1][1] = 2.0f/(t-b);
    scale[2][2] = 2.0f/(n-f);

    Matrix traslate = Matrix::identity(4);
    traslate[0][3] = -(r+l)/2.0f;
    traslate[1][3] = -(t+b)/2.0f;
    traslate[2][3] = -(n+f)/2.0f;

    MAT_OPROJECT = scale*traslate*MAT_OPROJECT;
}

void SetOrthogonalProjectMat(int width, int height, int depth) {
    MAT_OPROJECT = Matrix::identity(4);
    Matrix scale = Matrix::identity(4);
    scale[0][0] = 2.0f/width;
    scale[1][1] = 2.0f/height;
    scale[2][2] = 2.0f/depth;

    Matrix traslate = Matrix::identity(4);

    MAT_OPROJECT = scale*traslate*MAT_OPROJECT;
}

// 计算屏幕坐标
// x: [-1,1] -> [x,x+w]
// y: [-1,1] -> [y,y+h]
// z: [-1,1] -> [0,depth]
void SetScreenMat(int x, int y, int w, int h, int depth) {
    MAT_SCREEN = Matrix::identity(4);
    MAT_SCREEN[0][0] = w/2.f;
    MAT_SCREEN[1][1] = h/2.f;
    MAT_SCREEN[2][2] = depth/2.f;

    MAT_SCREEN[0][3] = x+w/2.f;
    MAT_SCREEN[1][3] = y+h/2.f;
    MAT_SCREEN[2][3] = depth/2.f;
}

void SetCamera(bool isVNormalized) {
    if (isVNormalized) {    
        // 模型的顶点数据已经归一化
        MAT_TRANS = MAT_SCREEN*MAT_PPROJECT*MAT_VIEW*MAT_MODEL;
    } else {
        // 这里针对没有归一化的顶点数据还存在问题
        MAT_TRANS = MAT_SCREEN*MAT_OPROJECT*MAT_PPROJECT*MAT_VIEW*MAT_MODEL;
    }
}

Vec3f barycentric(Vec4f *pts, Vec2f P) {
    Vec3f u = (Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x))^(Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void Rendering(RenderNode &rn) {
    Model *model = rn.model;
    Shader *shader = rn.shader;
    Log *log = rn.log;

    Vec4f screen_coords[3];
    int nfaces = model->nfaces();

    // #pragma omp parallel for
    // 未安装 OpenMP
    for (int i = 0; i < nfaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            InVectex inV;
            inV.v = v;
            inV.index = j;
            OutVectex outV;
            shader->vertex(inV, outV);
            screen_coords[j] = outV.sCoord;
            // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
        }
        DrawTriangle(screen_coords, rn);
        if (log && log->flag) log->show(i, nfaces);
    }
    if (log && log->flag) log->show(nfaces, nfaces);
}

void DrawTriangle(Vec4f *points, RenderNode &rn) {
    Shader *shader = rn.shader;
    Frame *frame = rn.frame;
    Zbuffer *zbuffer = rn.zbuffer;

    int width = frame->width, height = frame->height;
    Vec2f bboxmin(width-1, height-1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(width-1, height-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::min(bboxmin.x, points[i].x);
        bboxmin.y = std::min(bboxmin.y, points[i].y);
        bboxmax.x = std::max(bboxmax.x, points[i].x);
        bboxmax.y = std::max(bboxmax.y, points[i].y);
    }
    bboxmin.x = std::min(0.f, bboxmin.x);
    bboxmin.y = std::min(0.f, bboxmin.y);
    bboxmax.x = std::max(clamp.x, bboxmax.x);
    bboxmax.y = std::max(clamp.y, bboxmax.y);

    Vec2i P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc = barycentric(points, Vec2f(P.x, P.y));
            float z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
            float w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
            float depth = std::max(0.f, std::min(1.0f, z/w));
            if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer->get(P.x, P.y) > z) continue;
            // if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            InFragment in;
            OutFragment out;
            in.bar = bc;
            in.depth = z;
            if (!shader->fragment(in, out)) {
                zbuffer->set(P.x, P.y, z);
                frame->set(P.x, P.y, out.color);
            }
        }
    }
}

// 绘制该帧
void DrawFrame(Frame &frame, const char *filename) {
    int width = frame.width, height = frame.height;
    auto data = (unsigned char*)malloc(width*height*3);
    for (int i = 0; i < width*height; ++i) {
        for (int j = 0; j < 3; ++j) {
            data[i*3+j] = (unsigned char)(255*std::max(0.0f, std::min(1.0f, frame.buffer[i][j])));
        }
    }
    stbi_write_png(filename, width, height, 3, data, 0);
}

// 垂直翻转
void FlipFrame(Frame &frame) {
    int width = frame.width, height = frame.height;
    for (int i = 0; i < height / 2; ++i) {
        for (int j = 0; j < width; ++j) {
            Vec3f t = frame.buffer[j+width*i];
            frame.buffer[j+width*i] = frame.buffer[j+width*(height-1-i)];
            frame.buffer[j+width*(height-1-i)] = t;
        }
    }
}
}


// // 重心坐标计算
// Vec3f barycentric(Vec4f *pts, Vec2f P) {
//     Vec3f u = (Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x))^(Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
//     if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
//     return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
// }

// /* 
//  *  作用：根据 给定点ABC 绘制三角形。
//  *  参数：
//  *      pts: 点 ABC 坐标
//  *      shader: 该三角形的shader类型
//  *      image: TGA图片对象
//  *      zbuffer: z-buffer数据
//  *  返回：无
//  */
// void triangle(Vec4f *pts, Shader &shader, TGAImage &image, float *zbuffer) {
//     int iwidth = image.get_width(), iheight = image.get_height();
//     Vec2f bboxmin(iwidth-1,  iheight-1); 
//     Vec2f bboxmax(0, 0); 
//     Vec2f clamp(iwidth-1, iheight-1); 
//     for (int i=0; i<3; i++) { 
//         bboxmin.x = std::min(bboxmin.x, pts[i].x);
//         bboxmin.y = std::min(bboxmin.y, pts[i].y);
//         bboxmax.x = std::max(bboxmax.x, pts[i].x);
//         bboxmax.y = std::max(bboxmax.y, pts[i].y);
//     }
//     bboxmin.x = std::min(0.f, bboxmin.x);
//     bboxmin.y = std::min(0.f, bboxmin.y);
//     bboxmax.x = std::max(clamp.x, bboxmax.x);
//     bboxmax.y = std::max(clamp.y, bboxmax.y);

//     Vec2i P;
//     TGAColor color;
//     for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
//         for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
//             Vec3f bc = barycentric(pts, Vec2f(P.x, P.y));
//             float z = pts[0][2]*bc.x+pts[1][2]*bc.y+pts[2][2]*bc.z;
//             float w = pts[0][3]*bc.x+pts[1][3]*bc.y+pts[2][3]*bc.z;
//             // int depth = std::max(0, std::min(255, int(z/w+.5f)));
//             // float depth = std::max(0.f, std::min(1.0f, z/w));
//             if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer[P.x+iwidth*P.y] > z) continue;
//             if (!shader.fragment(bc, color)) {
//                 // zbuffer.set(P.x, P.y, depth);
//                 zbuffer[P.x+iwidth*P.y] = z;
//                 image.set(P.x, P.y, color);
//             }
//         }
//     }
// }


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

/*

void my_triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    int min_x = std::min(t0.x, std::min(t1.x, t2.x));
    int max_x = std::max(t0.x, std::max(t1.x, t2.x));
    int min_y = std::min(t0.y, std::min(t1.y, t2.y));
    int max_y = std::max(t0.y, std::max(t1.y, t2.y));

    Vec2i dir0 = t1 - t0;
    Vec2i dir1 = t2 - t1;
    Vec2i dir2 = t0 - t2;

    // 当斜率为0的时候，不能取1，算的时候y值会超出最大值
    // if (dir0.x == 0) {
    //     dir0.x = 1;
    // }
    // if (dir1.x == 0) {
    //     dir0.x = 1;
    // }
    // if (dir2.x == 0) {
    //     dir0.x = 1;
    // }

    // int limit_k = 1;
    // if (std::abs(dir0.x) <= limit_k) {
    //     int by = std::min(t0.y, t1.y);
    //     int ty = std::max(t0.y, t1.y);
    //     line(Vec2i(t0.x, by), Vec2i(t0.x, ty), image, color);
    //     line(Vec2i(t0.x-1, by), Vec2i(t0.x-1, ty), image, color);
    //     // line(Vec2i(t0.x+1, by), Vec2i(t0.x+1, ty), image, color);
    // }
    // if (std::abs(dir1.x) <= limit_k) {
    //     int by = std::min(t1.y, t2.y);
    //     int ty = std::max(t1.y, t2.y);
    //     line(Vec2i(t1.x, by), Vec2i(t1.x, ty), image, color);
    //     line(Vec2i(t1.x-1, by), Vec2i(t1.x-1, ty), image, color);
    //     // line(Vec2i(t1.x+1, by), Vec2i(t1.x+1, ty), image, color);
    // }
    // if (std::abs(dir2.x) <= limit_k) {
    //     int by = std::min(t2.y, t0.y);
    //     int ty = std::max(t2.y, t0.y);
    //     line(Vec2i(t2.x, by), Vec2i(t2.x, ty), image, color);
    //     line(Vec2i(t2.x-1, by), Vec2i(t2.x-1, ty), image, color);
    //     // line(Vec2i(t2.x+1, by), Vec2i(t2.x+1, ty), image, color);
    // }

    // 未考虑 dir.x 为零的情况
    float k0 = dir0.y * 1.0f / dir0.x;
    float k1 = dir1.y * 1.0f / dir1.x;
    float k2 = dir2.y * 1.0f / dir2.x;

    // std::cout << k0 << " " << k1 << " " << k2 << std::endl;

    for (int x = min_x; x <= max_x; ++x) {
        int y0 = ceil(k0 * (x - t0.x) + t0.y);
        int y1 = ceil(k1 * (x - t1.x) + t1.y);
        int y2 = ceil(k2 * (x - t2.x) + t2.y);
        int by = -1, ty = -1;
        if (y0 >= min_y && y0 <= max_y) {
            by = y0;
        }
        if (y1 >= min_y && y1 <= max_y) {
            if (by == -1) {
                by = y1;
            } else {
                ty = y1;
            }
        }
        if (y2 >= min_y && y2 <= max_y) {
            ty = y2;
        }
        if (ty < by) {
            std::swap(by, ty);
        }
        // if (by == -1 || ty == -1) {
        //     std::cout << min_y << " " << max_y << std::endl;
        //     std::cout << y0 << " " << y1 << " " << y2 << std::endl;
        //     std::cout << by << " " << ty << std::endl;
        //     std::cout << std::endl;
        // }
        line(Vec2i(x, by), Vec2i(x, ty), image, color);
    }
}

*/