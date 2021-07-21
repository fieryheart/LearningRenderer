#include "PathTracing.h"
#include <random>
#include <iostream>
#include <omp.h>

namespace QGL {

int PT_NUM_THREADS = 8;
Vec4f BACKGROUND_COLOR = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

int PATH_TRACING_N = 100;
float PATH_TRACING_P_RR = 0.9;
// std::mt19937 PATH_TRACING_RNG;
// std::uniform_real_distribution<float> PATH_TRACING_UNIFORM_DIST = std::uniform_real_distribution<float>(0, 1);

// 随机发生器
std::default_random_engine random(time(NULL));
std::uniform_real_distribution<float> dist01(0.0, 1.0);
std::uniform_real_distribution<float> dist11(-1.0, 1.0);
Randomizer<std::uniform_real_distribution<float>, float> random01(random, dist01);
Randomizer<std::uniform_real_distribution<float>, float> random11(random, dist11);


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
void PathTracing(PTNode &ptn) {

    // 初始化随机种子
    std::mt19937 PATH_TRACING_RNG;
    PATH_TRACING_RNG.seed(std::random_device()());

    Frame *frame = ptn.frame;
    Vec3f pos = ptn.camera;
    int width = ptn.width;
    int height = ptn.height;
    float fov = ptn.fov;
    int bound = ptn.bound;
    // log
    int omp_count = 0;
    Log log = Log(true, "PathTracing: ");

    // 构建BVH
    BVHBuilder *bvh = new BVHBuilder(ptn.models);
    std::cout << "bvh-triangles: " << bvh->tris.size() << std::endl;

    // 每个像素发射 PATH_TRACING_N 条光线
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
                // 随机选择单位像素框的某个位置点
                x = i + random01.get();
                y = j + random01.get();

                x = (2.0f*(x/(float)width)-1.0f)*tan(fov/2)*width/height;
                y = -(2.0f*(y/(float)height)-1.0f)*tan(fov/2);  // y 在数据中向下为正
                dir = (Vec3f(x, y, -1)).normalize();
                ray.dir = dir;

                // 光线碰撞判断
                if (RayInteract(bvh, ray, index, bc, p)) {
                    // printf("%d %d\n", i, j);

                    // 单个光线路径追踪
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
            omp_count++;
            log.show(omp_count, height);
        }
    }
}


//
// 参数：
//      bvh
//      index: bvh中三角形索引
//      p: 空间坐标点
//      bc: 重心系数
//
Vec4f RayTracing(BVHBuilder *bvh, Ray &ray, int index, Vec3f bc, Vec3f p) {
    Vec3f lp, next_p, N_p, N_lp;
    BVHTriangle *tri = bvh->tris[index];
    float k_r = 0.0f, f_r = 0.0f, L_i = 0.0f, dis = 0.0f;
    Vec4f lcolor, diffColor, L_e = Vec4f(0.0f), L_dir = Vec4f(0.0f), L_indir = Vec4f(0.0f);
    Vec3f w0, w1, tmp_w;

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
        // k_r = L_i*f_r/(ray.pos-p).norm()/lm->pdf();

        // 获取 光线方向
        Vec3f light_dir = (Vec3f(0, -0.5, -1)).normalize();
        if (-light_dir * ray.dir < 0) {
            k_r = 0.0f;
        } else {
            k_r = L_i*f_r/(ray.pos-p).norm();
            k_r = std::clamp(k_r, 0.0f, 1.0f);
        }
        L_e = lcolor*k_r;
        // std::cout << lcolor << " " << k_r << std::endl;
    }
    // return L_e;

    /*
    * 
    *  Contribution from the light source
    *
    */
    L_i = bvh->light->emit();
    lcolor = bvh->light->getColor();
    float diff_k = 0.7, light_k = 0.3;
    if (tri->model->type() == MDT_Buildin) {
        BuildinModel *bm = dynamic_cast<BuildinModel*>(tri->model);
        N_p = bm->norm(tri->nthface, 0);
        p = p+N_p*0.05f;
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
            // if (dis < 20.0f) dis += 20.0f;
            f_r = bm->brdf(tri->nthface, p, w0, -w1);
            k_r = L_i * f_r * -(N_p*w1) * (N_lp*w1) / dis / bvh->light->pdf();
            
            k_r = std::clamp(k_r, 0.0f, 1.0f);
            // printf("%f\n", k_r);
            bm->sampleDiffuse(tri->nthface, diffColor);
            
            
            // if (diffColor[0] == 1.0f && diffColor[1] == 1.0f && diffColor[2] == 1.0f) {
            //     diff_k = 0.f;
            //     light_k = 1.0f;
            // }
            // L_dir = (diffColor*diff_k + lcolor*light_k)*k_r;
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
        sm->sampleDiffuse(tri->nthface, bc, diffColor);

        // 只有一个光源，暂时先这么写～
        if (tmpIdx != -1 && bvh->tris[tmpIdx]->model->type() == MDT_Light) {
            dis = (p-lp).norm2();
            // if (dis < 20.0f) dis += 20.0f;
            f_r = sm->brdf(tri->nthface, p, w0, -w1);
            // k_r = L_i * f_r * (N_p*w0) * (N_lp*w1) / dis / bvh->light->pdf();
            // if (-(N_p*w1) < 0) {
            //     k_r = L_i * f_r * (N_p*w1) * (N_lp*w1) / dis / bvh->light->pdf();
            // } else {
            k_r = L_i * f_r * -(N_p*w1) * (N_lp*w1) / dis / bvh->light->pdf();
            // }

            k_r = std::clamp(k_r, 0.0f, 1.0f);
            // if (diffColor[0] == 1.0f && diffColor[1] == 1.0f && diffColor[2] == 1.0f) {
            //     diff_k = 0.f;
            //     light_k = 1.0f;
            // }
            // L_dir = (diffColor*diff_k + lcolor*light_k)*k_r;
            L_dir = diffColor*k_r;
        }

        // k_r = std::clamp(k_r, 0.1f, 1.0f);
        // L_dir = diffColor*k_r;
    }


    /*
    * 
    *  Contribution from other reflections.
    *
    */
    float ksi = random01.get();
    if (ksi > PATH_TRACING_P_RR) return L_e + L_dir;

    // Randomly choose ONE direction wi~pdf(w)
    Vec3f tmp = (Vec3f(random11.get(), random11.get(), random11.get())).normalize();
    Vec3f dir;
    if (N_p*tmp < 0) {
        dir = (N_p + tmp + N_p).normalize();
    } else if (N_p*tmp == 0) {
        dir = N_p;
    } else {
        dir = tmp;
    }
    Ray next_ray = Ray(p, dir);

    index = -1;
    RayInteract(bvh, next_ray, index, bc, next_p);
    if (index == -1 || bvh->tris[index]->model->type() == MDT_Light) return L_e + L_dir;


    tmp_w = next_p - p;
    // 
    float indir_k = (N_p*tmp_w) / PATH_TRACING_P_RR;
    indir_k = std::clamp(indir_k, 0.0f, 1.0f);
    // if (!isDir) indir_k = 1.0f;

    L_indir = RayTracing(bvh, next_ray, index, bc, next_p);
    L_indir = L_indir * indir_k;

    // if (L_e[0] == 0.0f && L_e[1] == 0.0f && L_e[2] == 0.0f
    // &&  L_dir[0] == 0.0f && L_dir[1] == 0.0f && L_dir[2] == 0.0f) {
    //     // std::cout << L_indir;
    //     // std::cout << (N_p*w0) / PATH_TRACING_P_RR << std::endl;
    // }

    // L_indir = RayTracing(bvh, next_ray, index, bc, next_p) * (N_p*w0) / PATH_TRACING_P_RR;
    // L_indir[0] = std::clamp(L_indir[0], 0.0f, 1.0f);
    // L_indir[1] = std::clamp(L_indir[1], 0.0f, 1.0f);
    // L_indir[2] = std::clamp(L_indir[2], 0.0f, 1.0f);

    return L_e + L_dir + L_indir;
    // return L_e + L_dir;
}


}