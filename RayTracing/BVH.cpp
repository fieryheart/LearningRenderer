#include "BVH.h"

#define MY_EPS 1e-5

namespace QGL {

// Axis Aligned Bounding Box
AxisAlignedBoundBox::AxisAlignedBoundBox(float minx, float maxx, float miny, float maxy, float minz, float maxz) {
    minn[0] = minx, maxn[0] = maxx;
    minn[1] = miny, maxn[1] = maxy;
    minn[2] = minz, maxn[2] = maxz;
    if (minx == maxx) maxn[0] += 0.1f;
    if (miny == maxy) maxn[1] += 0.1f;
    if (minz == maxz) maxn[2] += 0.1f;
}

void AxisAlignedBoundBox::interactAxis(int index, float p, float v, float &tin, float &tout) {
    if (std::abs(v) < 1e-5) {
        tin = -std::numeric_limits<float>::max();
        tout = std::numeric_limits<float>::max();
    } else {
        tin = (minn[index]-p)/v;
        tout = (maxn[index]-p)/v;
        if (v < 0) std::swap(tin, tout);
    }
}

bool AxisAlignedBoundBox::interact(Ray &ray) {
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
BVHTriangle::BVHTriangle(Model *model_, int nthface_) {
    model = model_;
    nthface = nthface_;
    minP = Vec3f(std::numeric_limits<float>::max());
    maxP = Vec3f(-std::numeric_limits<float>::max());
    for (int i = 0; i < 3; ++i) {
        minP[i] = std::min(minP[i], model->vert(nthface, 0)[i]);
        minP[i] = std::min(minP[i], model->vert(nthface, 1)[i]);
        minP[i] = std::min(minP[i], model->vert(nthface, 2)[i]);
        maxP[i] = std::max(maxP[i], model->vert(nthface, 0)[i]);
        maxP[i] = std::max(maxP[i], model->vert(nthface, 1)[i]);
        maxP[i] = std::max(maxP[i], model->vert(nthface, 2)[i]);
        centroid[i] = (minP[i] + maxP[i]) / 2.0f;
        
        // if (i == 2) {
        //     std::cout << model->vert(nthface, 0)[i] << std::endl;
        //     std::cout << model->vert(nthface, 1)[i] << std::endl;
        //     std::cout << model->vert(nthface, 2)[i] << std::endl;
        // }

        vert[i] = model->vert(nthface, i);
    }
    // std::cout << maxP[0] << " " << maxP[1] << " " << maxP[2] << std::endl;
    // normal = (mode->norm(nthface,0)+mode->norm(nthface,1)+mode->norm(nthface,2))/3.0f
}

void BVHTriangle::interact(InInteract &in, OutInteract &out) {
    Ray ray = in.ray;
    float t = in.t;

    Vec3f e1, e2, s, s1, s2;
    float tt, b1, b2;
    e1 = vert[1]-vert[0];
    e2 = vert[2]-vert[0];
    s = ray.pos - vert[0];
    s1 = (ray.dir)^e2;
    s2 = s^e1;
    float se = s1*e1;
    tt = s2*e2/se;
    b1 = s1*s/se;
    b2 = s2*(ray.dir)/se;

    Vec3f p = ray.launch(tt);

    // Log
    // if (in.idx == 1 && p.x == 0.5f) {
        // Vec3f dir = ray.dir;
        // std::cout << "(x, y): (" << dir.x << ", " << dir.y << ")";
        // if (b1 < 0 || b2 < 0 || 1-b1-b2 < 0) {
        //     std::cout << " tt: " << tt << " b1: " << b1 << " b2: " << b2 << std::endl;
        //     std::cout << p;
        // }
    // }


    // 当写成 b2>=0 或 1-b1-b2>=0时 会出现边界掉色，如example/error/PT_error_0
    // 不判断 tt > 0，如example/error/PT_error_[1|2|3]
    if (tt > 0 && tt < t && b1 >= -MY_EPS && b2 >= -MY_EPS && 1-b1-b2 >= -MY_EPS) {
        in.t = tt;
        out.idx = in.idx;
        out.t = tt;
        out.bc.x = 1-b1-b2;
        out.bc.y = b1;
        out.bc.z = b2;
    }
}

int BVHNode::getaabb(std::vector<BVHTriangle*> &objects, int left, int right) {
   // get axis to sort
    float minL[3] = {std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max()};
    float maxL[3] = {-std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max(),
                     -std::numeric_limits<float>::max()};
    for (int i = left; i <= right; ++i) {
        minL[0] = std::min(minL[0], objects[i]->minP[0]);
        minL[1] = std::min(minL[1], objects[i]->minP[1]);
        minL[2] = std::min(minL[2], objects[i]->minP[2]);
        maxL[0] = std::max(maxL[0], objects[i]->maxP[0]);
        maxL[1] = std::max(maxL[1], objects[i]->maxP[1]);
        maxL[2] = std::max(maxL[2], objects[i]->maxP[2]);
    }
    aabb = AxisAlignedBoundBox(minL[0], maxL[0], minL[1], maxL[1], minL[2], maxL[2]);

    int axis = 0; // 0为x， 1为y, 2为z
    if((maxL[1]-minL[1]) > (maxL[axis]-minL[axis])) axis = 1;
    if((maxL[2]-minL[2]) > (maxL[axis]-minL[axis])) axis = 2;
    return axis;
}

int BVHNode::BVHSort(std::vector<BVHTriangle*> &objects, int left, int right, int axis) {
    // sort
    BVHTriangle* tmp = NULL;
    if (objects[left]->centroid[axis] > objects[right]->centroid[axis]) {
        tmp = objects[left];
        objects[left] = objects[right];
        objects[right] = tmp;
    }

    float pivot = objects[left]->centroid[axis];
    int l = left + 1, r = right;
    while(l < r) {
        while (l < r && objects[l]->centroid[axis] < pivot) l++;
        while (l < r && objects[r]->centroid[axis] >= pivot) r--;
        if (l < r) {
            tmp = objects[l];
            objects[l] = objects[r];
            objects[r] = tmp;
        }
    }
    if (l == r) {
        tmp = objects[left];
        objects[left] = objects[r-1];
        objects[r-1] = tmp;
    }
    return r-1;
}

//
BVHNode::BVHNode(std::vector<BVHTriangle*> &objects, int left, int right) {
    
    // Log
    // std::cout << "BVHNode: " << left << " " << right << std::endl;
    
    
    if (left > right) {
        aabb = AxisAlignedBoundBox(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
        lChild = rChild = NULL;
        oleft = oright = -1;
    } else if (right-left+1 <= 5) {
        getaabb(objects, left, right);
        oleft = left, oright = right;
        lChild = rChild = NULL;
    } else {
        int axis = getaabb(objects, left, right);
        int mid = BVHSort(objects, left, right, axis);
        lChild = new BVHNode(objects, left, mid);
        rChild = new BVHNode(objects, mid+1, right);
        oleft = oright = -1;
    }
}

//
void BVHNode::interact(Ray &ray, std::vector<int> &indices) {
    // Log
    // if ((ray.dir.x < 0 || ray.dir.y < 0) && aabb.interact(ray)) {
    //     std::cout << aabb;
    //     std::cout << aabb.interact(ray) << std::endl;
    // }
    if (aabb.interact(ray)) {
        if (lChild) lChild->interact(ray, indices);
        if (rChild) rChild->interact(ray, indices);
        if (oleft >= 0) {
            // std::cout << oleft << " " << oright << std::endl;
            for (int i = oleft; i <= oright; ++i) {
                indices.push_back(i);
            }
        }
    }
}

void BVHNode::deleteBVHNode(BVHNode* root) {
    if (root) {
        if (root->lChild) deleteBVHNode(root->lChild);
        if (root->rChild) deleteBVHNode(root->rChild);
        delete root;
    }
}

//
BVHBuilder::BVHBuilder(std::vector<Model*> models) {
    std::cout << "BVHBuilder" << std::endl;

    for (int i = 0; i < models.size(); ++i) {
        for (int j = 0; j < models[i]->nfaces(); ++j) {
            BVHTriangle *tri = new BVHTriangle(models[i], j);
            tris.push_back(tri);
        }
        if (models[i]->type() == MDT_Light) {
            light = dynamic_cast<LightModel*>(models[i]);
        }
    }

    root = new BVHNode(tris, 0, (int)tris.size()-1);
}

//
BVHBuilder::~BVHBuilder() {
    BVHNode::deleteBVHNode(root);
}

//
void BVHBuilder::interact(Ray &ray, std::vector<int> &indices) {
    if (root) root->interact(ray, indices);
}

}