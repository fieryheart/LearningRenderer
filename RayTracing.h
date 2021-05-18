#ifndef __RAYTRACING_H__
#define __RAYTRACING_H__

#include "objects.h"
#include "model.h"

namespace QGL {

struct InInteract{
    Ray ray;
    int idx;
    float t;
    InInteract() {}
};

struct OutInteract {
    int idx;
    float t;
    Vec3f bc;
    OutInteract() {}
};

struct AxisAlignedBoundBox{
    float minn[3], maxn[3];
    AxisAlignedBoundBox() {}
    AxisAlignedBoundBox(float minx, float maxx, float miny, float maxy, float minz, float maxz);
    ~AxisAlignedBoundBox() {}
    void interactAxis(int index, float p, float v, float &tin, float &tout);
    bool interact(Ray &ray);
};

std::ostream& operator<<(std::ostream& s, AxisAlignedBoundBox& aabb);

struct BVHTriangle {
    Model *model;
    int nthface;

    Vec3f vert[3];
    Vec3f normal;

    Vec3f minP, maxP;
    Vec3f centroid;
    BVHTriangle() {}
    BVHTriangle(Model *model_, int nthface_);
    ~BVHTriangle() {}

    void interact(InInteract &in, OutInteract &out);
};

struct BVHNode {
    AxisAlignedBoundBox aabb;
    // std::vector<BVHTriangle*> objs;
    int oleft, oright;
    BVHNode *lChild;
    BVHNode *rChild;
    BVHNode() {}
    BVHNode(std::vector<BVHTriangle*> &objects, int left, int right);
    int getaabb(std::vector<BVHTriangle*> &objects, int left, int right);
    int BVHSort(std::vector<BVHTriangle*> &objects, int left, int right, int axis);
    void interact(Ray &ray, std::vector<int> &indices);

    static void deleteBVHNode(BVHNode* root);
};

struct BVHBuilder {
    BVHNode *root;
    std::vector<BVHTriangle*> tris;
    LightModel *light;
    BVHBuilder(){}
    BVHBuilder(std::vector<Model*> models);
    ~BVHBuilder();
    void interact(Ray &ray, std::vector<int> &indices);
};

}


#endif // __RAYTRACING_H__