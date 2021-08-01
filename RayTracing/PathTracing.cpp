#include "PathTracing.h"
#include <random>
#include <iostream>
#include <omp.h>

namespace QGL {

int PT_NUM_THREADS = 8;
Vec4f BACKGROUND_COLOR = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

int PATH_TRACING_N = 500;
float PATH_TRACING_P_RR = 0.5;
// std::mt19937 PATH_TRACING_RNG;
// std::uniform_real_distribution<float> PATH_TRACING_UNIFORM_DIST = std::uniform_real_distribution<float>(0, 1);

// 随机发生器
std::default_random_engine random(time(NULL));
std::uniform_real_distribution<float> dist01(0.0, 1.0);
std::uniform_real_distribution<float> dist11(-1.0, 1.0);
Randomizer<std::uniform_real_distribution<float>, float> random01(random, dist01);
Randomizer<std::uniform_real_distribution<float>, float> random11(random, dist11);

Vec2f Hammersley(uint32_t i, uint32_t N) { // 0-1
    uint32_t bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return {float(i) / float(N), rdi};
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
                    one_ray_color = PTShading(bvh, ray, index, bc, p);
                    one_ray_color = vpow(one_ray_color, 0.45);
                    one_ray_color[0] = std::clamp(one_ray_color[0], 0.0f, 1.0f);
                    one_ray_color[1] = std::clamp(one_ray_color[1], 0.0f, 1.0f);
                    one_ray_color[2] = std::clamp(one_ray_color[2], 0.0f, 1.0f);
                } else {
                    one_ray_color = Vec4f(0.0f);
                }

                // color = color + one_ray_color / (PATH_TRACING_N*1.0f);
                one_ray_color = vpow(one_ray_color, 2.2);
                color = color + one_ray_color;
            }
            color = color / (PATH_TRACING_N*1.0f);
            color = vpow(color, 0.45);
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
Vec4f PTShading(BVHBuilder *bvh, Ray &ray, int index, Vec3f bc, Vec3f p) {
    Vec3f l_p, next_p, N_p = Vec3f(1,0,0), N_l = Vec3f(1,0,0);
    BVHTriangle *tri = bvh->tris[index];
    float dis = 0.0f;
    Vec4f color_l, color_diff, f_r, L_i = Vec4f(0.0f), L_e = Vec4f(0.0f), L_dir = Vec4f(0.0f), L_indir = Vec4f(0.0f);
    Vec3f wi, wo;

    /*
    * 
    *  Light Source
    *
    */
    if (tri->model->type() == MDT_Light) {
        LightModel *lm = dynamic_cast<LightModel*>(tri->model);
        L_i = lm->emit();
        f_r = 1 / M_PI;

        // 获取光源法向量 N_l
        // 判断是否打到光源
        // Le = I*fd*cos(N_l, light_p-camrea)
        // fd = color_l / Pi
        
        // Vec3f light_dir = (Vec3f(0, -0.5, -1)).normalize();
        // if (-light_dir * ray.dir < 0) {
        //     k_r = 0.0f;
        // } else {
        //     k_r = L_i*f_r*();
        //     k_r = std::clamp(k_r, 0.0f, 1.0f);
        // }
    }

    /*
    * 
    *  Direct Illumination
    *
    */
    if (tri->model->type() == MDT_Buildin) {
        BuildinModel *bm = dynamic_cast<BuildinModel*>(tri->model);
        N_p = bm->norm(tri->nthface, 0);
        // std::cout << "buildin-np: " << N_p;

        p = p+N_p*0.05f;
        L_i = bvh->light->emit();
        l_p = bvh->light->randomSample(); // uniformly sample the light.
        N_l = bvh->light->norm(0, 0);
        // w0 = (ray.pos-p).normalize();
        // w1 = (p-lp).normalize();
        wi = (l_p-p).normalize();
        wo = -ray.dir;

        // occlusion
        Ray trashRay = Ray(p, wi);
        int trashIdx = -1;
        Vec3f trashBC, tmpP;
        RayInteract(bvh, trashRay, trashIdx, trashBC, tmpP);

        bm->sampleDiffuse(tri->nthface, color_diff);
        // 只有一个光源，暂时先这么写～
        if (trashIdx != -1 && bvh->tris[trashIdx]->model->type() == MDT_Light) {
            dis = (p-l_p).norm2();
            
            color_diff = vpow(color_diff, 2.2);
            f_r = color_diff * bm->brdf(tri->nthface, p, wi, wo);

            float cosTheta0 = std::max(0.0f, dot(N_p, wi));
            float cosTheta1 = std::max(0.0f, -dot(N_l, wi));
            L_dir = L_i * f_r * cosTheta0 * cosTheta1 / dis / bvh->light->pdf();
        }

        // environment light
        // L_dir = L_dir + vpow(vpow(diffColor, 2.2) * 0.05, 0.45);

    } else if (tri->model->type() == MDT_Strange) {
        StrangeModel *sm = dynamic_cast<StrangeModel*>(tri->model);
        N_p = sm->norm(tri->nthface, bc);
        // std::cout << "strange-np: " << N_p;

        p = p + N_p*0.5f;
        L_i = bvh->light->emit();
        l_p = bvh->light->randomSample(); // uniformly sample the light.
        N_l = bvh->light->norm(0, 0);
        wi = (l_p-p).normalize();
        wo = -ray.dir;

        // occlusion
        Ray trashRay = Ray(p, wi);
        int trashIdx = -1;
        Vec3f trashBC, tmpP;
        RayInteract(bvh, trashRay, trashIdx, trashBC, tmpP);
        
        sm->sampleDiffuse(tri->nthface, bc, color_diff);
        // 只有一个光源，暂时先这么写～
        if (trashIdx != -1 && bvh->tris[trashIdx]->model->type() == MDT_Light) {
            dis = (p-l_p).norm2();
            
            f_r = color_diff * sm->brdf(tri->nthface, p, wi, wo);

            float cosTheta0 = std::max(0.0f, dot(N_p, wi));
            float cosTheta1 = std::max(0.0f, -dot(N_l, wi));
            L_dir = L_i * f_r * cosTheta0 * cosTheta1 / dis / bvh->light->pdf();
        }
    }
    

    /*
    * 
    *  Indirect Illumination
    *
    */
    if (tri->model->type() != MDT_Light) {
        float ksi = random01.get();
        if (ksi > PATH_TRACING_P_RR) return L_e + L_dir;

        // Randomly choose ONE direction wi~pdf(w)
        // 没有均匀采样
        Vec3f tmp = (Vec3f(random11.get(), random11.get(), random11.get())).normalize();
        Vec3f dir;
        if (dot(N_p, tmp) < 0) {
            dir = (N_p + tmp + N_p).normalize();
        } else if (dot(N_p, tmp) == 0) {
            dir = N_p;
        } else {
            dir = tmp;
        }

        Ray next_ray = Ray(p, dir);

        index = -1;
        RayInteract(bvh, next_ray, index, bc, next_p);
        if (index == -1 || bvh->tris[index]->model->type() == MDT_Light) return L_e + L_dir;

        wi = dir;
        float cosTheta = std::max(0.0f, dot(N_p, wi));
        float k_indir = 2 * cosTheta / PATH_TRACING_P_RR;
        // indir_k = std::clamp(indir_k, 0.0f, 1.0f);
        L_indir = PTShading(bvh, next_ray, index, bc, next_p) * color_diff * k_indir;
    }


    return L_e + L_dir + L_indir;
}
}