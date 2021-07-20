#include "PathTracing.h"
#include <random>
#include <iostream>
#include <omp.h>

namespace QGL {

int PT_NUM_THREADS = 8;
Vec4f BACKGROUND_COLOR = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

int PATH_TRACING_N = 10;
float PATH_TRACING_P_RR = 0.7;
std::mt19937 PATH_TRACING_RNG;
std::uniform_real_distribution<float> PATH_TRACING_UNIFORM_DIST = std::uniform_real_distribution<float>(0, 1);

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
void RenderingByPathTracing(PTNode &in) {

    // 初始化随机种子
    std::mt19937 PATH_TRACING_RNG;
    PATH_TRACING_RNG.seed(std::random_device()());

    BVHBuilder *bvh = new BVHBuilder(in.models);

    // std::cout << bvh->tris.size() << std::endl;

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

    Log log = Log(true, "PathTracing: ");
    int cnt = 0;

    #pragma omp parallel for num_threads(PT_NUM_THREADS)
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
            // color[0] = std::min(1.0f, color[0]);
            // color[1] = std::min(1.0f, color[1]);
            // color[2] = std::min(1.0f, color[2]);
            color[3] = 1.0f;
            frame->set(i, j, color);
        }

        #pragma omp critical
        {
            cnt++;
            log.show(cnt, height);
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
        // L_dir = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

        StrangeModel *sm = dynamic_cast<StrangeModel*>(tri->model);
        N_p = sm->norm(tri->nthface, bc);
        p = p + N_p*0.5f;
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
            f_r = sm->brdf(tri->nthface, p, w0, -w1);
            k_r = L_i * f_r * (N_p*w0) * (N_lp*w1) / dis / bvh->light->pdf();
            sm->sampleDiffuse(tri->nthface, bc, diffColor);
            
            // std::cout << L_i << " " << f_r << " " << N_p*w0 << " " << N_lp*w1 << " " << dis << " " << bvh->light->pdf() << std::endl;
            // std::cout << "N_p: " << N_p;
            // std::cout << "w0: " << w0;
            // std::cout << "k_r: " << k_r << " ";
            // std::cout << diffColor;
            L_dir = diffColor*k_r;
        }        

        return L_e + L_dir;
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