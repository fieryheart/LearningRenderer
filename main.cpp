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
const Vec3f light1_dir = Vec3f(0,0,-1);

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

Vec3f barycentric(Vec2i *pts, Vec2i P) { 
    Vec3f u = cross(Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x), Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color) { 
    Vec2i bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2i bboxmax(0, 0); 
    Vec2i clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::min(bboxmin.x, pts[i].x);
        bboxmin.y = std::min(bboxmin.y, pts[i].y);
        bboxmax.x = std::max(bboxmax.x, pts[i].x);
        bboxmax.y = std::max(bboxmax.y, pts[i].y);
    }
    bboxmin.x = std::min(0, bboxmin.x);
    bboxmin.y = std::min(0, bboxmin.y);
    bboxmax.x = std::max(clamp.x, bboxmax.x);
    bboxmax.y = std::max(clamp.y, bboxmax.y);

    Vec2i P; 
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            image.set(P.x, P.y, color); 
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

void start(TGAImage &image) {
    // Vec2i pts[3] = {Vec2i(100,100), Vec2i(100, 300), Vec2i(500, 200)}; 
    // my_triangle(pts[0], pts[1], pts[2], image, red);
    // triangle(pts, image, red);

    for (int i=0; i<model->nfaces(); i++) { 
        std::vector<int> face = model->face(i); 
        Vec2i screen_coords[3]; 
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) { 
            Vec3f v = model->vert(face[j]); 
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
            world_coords[j] = v;
        } 
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light1_dir;
        if (intensity>0) {
            my_triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
            // triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }    
}

int main(int argc, char** argv) {
    model = new Model("../obj/african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);
    start(image);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}