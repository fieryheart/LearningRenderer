#ifndef __RAYTRACING_H__
#define __RAYTRACING_H__

#include "objects.h"
#include "model.h"

namespace QGL {

struct AxisAlignedBoundBox : Object{
    float minn[3], maxn[3];
    AxisAlignedBoundBox() {}
    AxisAlignedBoundBox(float minx, float maxx, float miny, float maxy, float minz, float maxz);
    ~AxisAlignedBoundBox() {}
    void interactAxis(int index, float p, float v, float &tin, float &tout);
    bool interact(Ray &ray, float t);
};

struct BVHTriangle {
    Model *model;
    int nthface;

    Vec3f minP, maxP;
    Vec3f centroid;
    BVHTriangle() {}
    BVHTriangle(Model *model, int nthface);
    ~BVHTriangle() {}
};

struct BVHNode {
    AxisAlignedBoundBox aabb;
    int osize;
    std::vector<BVHTriangle*> objs;
    BVHNode *left;
    BVHNode *right;
    BVHNode() {}
    BVHNode(std::vector<BVHTriangle*> &objects);
    void interact(Ray &ray, std::vector<BVHTriangle*> &objects);
};

struct BVHBuilder {
    BVHNode *root;
    BVHBuilder(){}
    BVHBuilder(std::vector<Model*> models);
    ~BVHBuilder(){}
    void interact(Ray &ray, std::vector<BVHTriangle*> &objects);
};

}


#endif // __RAYTRACING_H__