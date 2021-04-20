#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth = 255;  // 根据深度值 计算颜色
const Vec3f light1_dir = Vec3f(1,-1,1).normalize();
float *zbuffer = new float[width*height];
const Vec3f camera = Vec3f(1,1,10);
const Vec3f origin = Vec3f(0,0,0);
Matrix ModelMatrix(4, 4);       // 模型空间
Matrix ViewMatrix(4, 4);        // 视角空间
Matrix ProjectMatrix(4, 4);     // 投影空间
Matrix ViewportMatrix(4, 4);    // 屏幕空间

/* 
 *  作用：根据 点A 和 点B 画线。
 *  参数：
 *      x0: 点A的x值
 *      y0: 点A的y值
 *      x1: 点B的x值
 *      y1: 点B的y值
 *      image: TGA图片对象
 *      color: TGA颜色
 *  返回：无
 */
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    int x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) {
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (y1>y0?1:-1); 
            error2 -= dx*2; 
        } 
    }     
}

Vec3f cross(Vec3f A, Vec3f B) {
    return Vec3f(A.y*B.z-A.z*B.y, A.z*B.x-A.x*B.z, A.x*B.y-A.y*B.x);
}

// v 表示点
Matrix vec2mat(Vec3f v) {
    Matrix mat(4, 1);
    mat[0][0] = v[0];
    mat[1][0] = v[1];
    mat[2][0] = v[2];
    mat[3][0] = 1.f;
    return mat;
}

Vec3f mat2vec(Matrix mat) {
    Vec3f v;
    v[0] = mat[0][0];
    v[1] = mat[1][0];
    v[2] = mat[2][0];
    return v;
}

void computeModelMatrix() {
    ModelMatrix = Matrix::identity(4);
}

void computeViewMatrix(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();
    ViewMatrix = Matrix::identity(4);
    Matrix mat = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        ViewMatrix[0][i] = x[i];
        ViewMatrix[1][i] = y[i];
        ViewMatrix[2][i] = z[i];

        mat[i][3] = -center[i];
    }
    
    ViewMatrix = ViewMatrix*mat;
}

void computeProjectMatrix() {
    // 透视投影
    ProjectMatrix = Matrix::identity(4);
    ProjectMatrix[3][2] = -1.f/(camera-origin).norm();
}

// x: [-1,1] -> [x,x+w]
// y: [-1,1] -> [y,y+h]
// z: [-1,1] -> [0,depth]
void computeViewportMatrix(int x, int y, int w, int h) {
    ViewportMatrix = Matrix::identity(4);
    ViewportMatrix[0][0] = w/2.f;
    ViewportMatrix[1][1] = h/2.f;
    ViewportMatrix[2][2] = depth/2.f;

    ViewportMatrix[0][3] = x+w/2.f;
    ViewportMatrix[1][3] = y+h/2.f;
    ViewportMatrix[2][3] = depth/2.f;
}

// 重心坐标计算
Vec3f barycentric(Vec3f *pts, Vec3f P) {
    Vec3f u = cross(Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x), Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

// 从texture中获取颜色
TGAColor getColorFromTexture(TGAImage &texture, Vec3f pos) {
    int tWidth = texture.get_width();
    int tHeight = texture.get_height();
    return texture.get(int(pos.x*tWidth), int(pos.y*tHeight));
}

/* 
 *  作用：根据 给定点ABC 绘制三角形。
 *  参数：
 *      pts: 点 ABC 坐标
 *      image: TGA图片对象
 *      texture: 纹理对象
 *      texture_coords: 点ABC对应的纹理坐标
 *  返回：无
 */
void triangle(Vec3f *pts, TGAImage &image, TGAImage &texture, Vec3f *texture_coords, float *intensity) { 
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2f bboxmax(0, 0); 
    Vec2f clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::min(bboxmin.x, pts[i].x);
        bboxmin.y = std::min(bboxmin.y, pts[i].y);
        bboxmax.x = std::max(bboxmax.x, pts[i].x);
        bboxmax.y = std::max(bboxmax.y, pts[i].y);
    }
    bboxmin.x = std::min(0.f, bboxmin.x);
    bboxmin.y = std::min(0.f, bboxmin.y);
    bboxmax.x = std::max(clamp.x, bboxmax.x);
    bboxmax.y = std::max(clamp.y, bboxmax.y);

    Vec3f P, tex_uv;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            if (int(P.x+width*P.y) < 0 || int(P.x+width*P.y) >= width*height) continue;
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = pts[0].z*bc_screen.x + pts[1].z*bc_screen.y + pts[2].z*bc_screen.z;
            // std::cout << int(color.r) << " " << int(color.g) << " " << int(color.b) << std::endl;
            // 应用zbuffer
            if (zbuffer[int(P.x+width*P.y)] < P.z) {
                zbuffer[int(P.x+width*P.y)] = P.z;
                tex_uv.x = texture_coords[0].x*bc_screen.x + texture_coords[1].x*bc_screen.y + texture_coords[2].x*bc_screen.z;
                tex_uv.y = texture_coords[0].y*bc_screen.x + texture_coords[1].y*bc_screen.y + texture_coords[2].y*bc_screen.z;
                float weight = (bc_screen[0]*intensity[0]+bc_screen[1]*intensity[1]+bc_screen[2]*intensity[2]);
                TGAColor color = getColorFromTexture(texture, tex_uv)*weight;
                image.set(P.x, P.y, color);
            }
        } 
    } 
} 

void triangle(Vec3f *pts, TGAImage &image, float *intensity) { 
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2f bboxmax(0, 0); 
    Vec2f clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::min(bboxmin.x, pts[i].x);
        bboxmin.y = std::min(bboxmin.y, pts[i].y);
        bboxmax.x = std::max(bboxmax.x, pts[i].x);
        bboxmax.y = std::max(bboxmax.y, pts[i].y);
    }
    bboxmin.x = std::min(0.f, bboxmin.x);
    bboxmin.y = std::min(0.f, bboxmin.y);
    bboxmax.x = std::max(clamp.x, bboxmax.x);
    bboxmax.y = std::max(clamp.y, bboxmax.y);

    Vec3f P, tex_uv;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            if (P.x>=width||P.y>=height||P.x<0||P.y<0) continue;
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = pts[0].z*bc_screen.x + pts[1].z*bc_screen.y + pts[2].z*bc_screen.z;
            // std::cout << int(color.r) << " " << int(color.g) << " " << int(color.b) << std::endl;
            // 应用zbuffer
            if (zbuffer[int(P.x+width*P.y)] < P.z) {
                zbuffer[int(P.x+width*P.y)] = P.z;
                TGAColor color = white*(bc_screen[0]*intensity[0]+bc_screen[1]*intensity[1]+bc_screen[2]*intensity[2]);
                image.set(P.x, P.y, color);
            }
        } 
    } 
} 

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

void start(TGAImage &image, TGAImage &texture) {
    // Vec2i pts[3] = {Vec2i(100,100), Vec2i(100, 300), Vec2i(500, 200)}; 
    // my_triangle(pts[0], pts[1], pts[2], image, red);
    // triangle(pts, image, red);

    // 初始化各转换矩阵
    computeModelMatrix();
    computeViewMatrix(camera, origin, Vec3f(0, 1, 0));
    computeProjectMatrix();
    computeViewportMatrix(width/8, height/8, width*3/4, height*3/4);

    // 初始化zbuffer
    for (int i = 0; i < width*height; ++i) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    Matrix mappingMatrix = ViewportMatrix*ProjectMatrix*ViewMatrix*ModelMatrix;

    int face_num = model->nfaces();
    // face_num = 270;
    for (int i=0; i<face_num; i++) { 

        std::vector<int> face = model->face(i); 
        std::vector<int> faceTex = model->faceTex(i);
        Vec3f screen_coords[3];     // 屏幕坐标
        Vec3f world_coords[3];      // 世界坐标
        Vec3f texture_coords[3];    // 纹理坐标
        float intensity[3];         // 当前点 光强

        for (int j=0; j<3; j++) { 
            Vec3f v = model->vert(face[j]); 
            Matrix mv = vec2mat(v);
            // v = v*(1.f/(1.f-v.z/camera_pos.z));

            // 需要进行MVP转换，将模型各个点映射到屏幕的坐标上
            // screen_coords[j] = Vec3f(int((v.x+1.)/2.*width+.5), int((v.y+1.)/2.*height+.5), v.z); 
            screen_coords[j] = mat2vec(mappingMatrix*mv);

            world_coords[j] = v;
            texture_coords[j] = model->texCoord(faceTex[j]);
            
            intensity[j] = model->norm(i, j)*light1_dir;
        }

        // my_triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        
        // 纹理插值
        triangle(screen_coords, image, texture, texture_coords, intensity);

        // 法向量插值
        // triangle(screen_coords, image, intensity);
    }    
}

int main(int argc, char** argv) {
    model = new Model("../obj/african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("../obj/african_head_diffuse.tga");
    start(image, texture);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    delete[] zbuffer;
    return 0;
}