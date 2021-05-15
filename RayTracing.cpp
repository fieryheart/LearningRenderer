#include <iostream>
#include "RayTracing.h"

namespace QGL {

// Axis Aligned Bounding Box
AxisAlignedBoundBox::AxisAlignedBoundBox(float minx, float maxx, float miny, float maxy, float minz, float maxz) {
    minn[0] = minx, maxn[0] = maxx;
    minn[1] = miny, maxn[1] = maxy;
    minn[2] = minz, maxn[2] = maxz;
}

void AxisAlignedBoundBox::interactAxis(int index, float p, float v, float &tin, float &tout) {
    if (std::abs(v) < 1e-5) {
        tin = -std::numeric_limits<float>::max();
        tout = -std::numeric_limits<float>::max();
    } else {
        tin = (minn[index]-p)/v;
        tout = (maxn[index]-p)/v;
        if (p > maxn[index]) std::swap(tin, tout);
    }
}

bool AxisAlignedBoundBox::interact(Ray &ray, float t) {
    float tin[3], tout[3];
    for (int i = 0; i < 3; ++i) {
        interactAxis(i, ray.pos[i], ray.dir[i], tin[i], tout[i]);
    }
    float in = std::max(tin[0], std::max(tin[1], tin[2]));
    float out = std::min(tout[0], std::min(tout[1], tout[2]));
    // std::cout << in << " " << out << std::endl;
    if (out > in && out >= 0) return true;
    else return false;
}

//
BVHTriangle::BVHTriangle(Model *model, int nthface) {

}

//
BVHNode::BVHNode(std::vector<BVHTriangle*> &objects) {

}

//
void BVHNode::interact(Ray &ray, std::vector<BVHTriangle*> &objects) {

}

//
BVHBuilder::BVHBuilder(std::vector<Model*> models) {
    
}

//
void BVHBuilder::interact(Ray &ray, std::vector<BVHTriangle*> &objects) {

}
}