#ifndef __PATHTRACING_H__
#define __PATHTRACING_H__

#include "BVH.h"

namespace QGL {

struct PTNode {
    Frame *frame;
    std::vector<Model*> models;
    Vec3f camera;
    int width;
    int height;
    float fov;
    int bound;
    // int light;
    PTNode(){}
};

// 路径追踪
void PathTracing(PTNode &in);
Vec4f RayTracing(BVHBuilder *bvh, Ray &ray, int index, Vec3f bc, Vec3f p);

}

#endif