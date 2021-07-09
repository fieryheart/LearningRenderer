#include "rasterizer.h"

namespace QGL {

int NUM_THREADS = 8;

Matrix MAT_MODEL = Matrix::identity(4);
Matrix MAT_VIEW = Matrix::identity(4);
Matrix MAT_PROJECT = Matrix::identity(4);
Matrix MAT_ORTHO_PROJECT = Matrix::identity(4);
Matrix MAT_SCREEN = Matrix::identity(4);     // 屏幕空间
Matrix MAT_TRANS = Matrix::identity(4);
Matrix MAT_NORM_TRANS = Matrix::identity(4);

void LookAt(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    std::cout << x;
    std::cout << y;
    std::cout << z;
    Matrix viewMatrix = Matrix::identity(4);
    Matrix mat = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        viewMatrix[0][i] = x[i];
        viewMatrix[1][i] = y[i];
        viewMatrix[2][i] = z[i];
        mat[i][3] = -eye[i];
    }
    MAT_VIEW = viewMatrix*mat;
}

void SetProjectMat(float fov, float ratio, float near, float far) {
    MAT_PROJECT = Matrix::identity(4);
    float d = 1/tan((fov/2)*M_PI/180);
    float A = -(near+far)/(far-near);
    float B = -2*far*near/(far-near);
    MAT_PROJECT[0][0] = d/ratio;
    MAT_PROJECT[1][1] = d;
    MAT_PROJECT[2][2] = A;
    MAT_PROJECT[2][3] = B;
    MAT_PROJECT[3][2] = -1;
}

// 计算屏幕坐标
// x: [-1,1] -> [x,x+w]
// y: [-1,1] -> [y,y+h]
// z: [-1,1] -> [0,depth]
void SetScreenMat(int x, int y, int w, int h, int depth) {
    MAT_SCREEN = Matrix::identity(4);
    MAT_SCREEN[0][0] = w/2.f;
    MAT_SCREEN[1][1] = h/2.f;
    MAT_SCREEN[2][2] = depth/2.f;

    MAT_SCREEN[0][3] = x+w/2.f;
    MAT_SCREEN[1][3] = y+h/2.f;
    MAT_SCREEN[2][3] = depth/2.f;
}

void Init() {
    MAT_TRANS = MAT_SCREEN*MAT_PROJECT*MAT_VIEW*MAT_MODEL;
    MAT_NORM_TRANS = MAT_MODEL.inverse().transpose();
}

Vec3f barycentric(Vec4f *pts, Vec2f P) {
    Vec3f u = (Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x))^(Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void Rasterize(RasterNode &rn) {
    if (rn.comType == CT_Single) SingleRasterize(rn);
    else if (rn.comType == CT_Multi) MultiRasterize(rn);
    else return;
}

void DrawTriangle(Vec4f *points, RasterNode &rn) {
    Model *model = rn.model;
    Shader *shader = rn.shader;
    Frame *frame = rn.frame;
    Zbuffer *zbuffer = rn.zbuffer;
    int width = frame->width, height = frame->height;

    Vec2f bboxmin(width-1, height-1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(width-1, height-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::min(bboxmin.x, points[i].x);
        bboxmin.y = std::min(bboxmin.y, points[i].y);
        bboxmax.x = std::max(bboxmax.x, points[i].x);
        bboxmax.y = std::max(bboxmax.y, points[i].y);
    }
    bboxmin.x = std::max(0.f, bboxmin.x);
    bboxmin.y = std::max(0.f, bboxmin.y);
    bboxmax.x = std::min(clamp.x, bboxmax.x);
    bboxmax.y = std::min(clamp.y, bboxmax.y);

    Vec2i P;
    Vec3f bc;
    float z, w, depth, weight;
    // std::cout << bboxmin;
    // std::cout << bboxmax;
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
        for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
            bc = barycentric(points, Vec2f(P.x, P.y));
            z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
            w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
            // z = z/w;
            // depth = std::max(0.f, std::min(1.0f, z/w));
            if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer->get(P.x, P.y) > z) continue;
            InFrag in;
            OutFrag out;
            in.bar = bc;
            in.depth = z;
            in.model = model;
            if (!shader->fragment(in, out)) {
                zbuffer->set(P.x, P.y, z);
                frame->set(P.x, P.y, out.color);
            }
        }
    }
}

void SingleRasterize(RasterNode &rn) {
    Model *model = rn.model;
    Shader *shader = rn.shader;
    Log *log = rn.log;
    // Zbuffer *zbuffer = rn.zbuffer;
	Frame *frame = rn.frame;
    Vec4f screen_coords[3];
    int nfaces = model->nfaces();
    for (int i = 0; i < nfaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            InVert inV;
            OutVert outV;

            inV.v = v;
            inV.nthface = i;
            inV.nthvert = j;
            inV.model = model;
            
            shader->vertex(inV, outV);
            screen_coords[j] = outV.sCoord;
            // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
        }

        DrawTriangle(screen_coords, rn);

        if (log && log->flag) log->show(i, nfaces);
    }
    if (log && log->flag) log->show(nfaces, nfaces);
}

void MultiRasterize(RasterNode &rn) {
    std::cout << "Multithreading rasterize is not supported~" << std::endl;

    Model *model = rn.model;
    int nfaces = model->nfaces();

    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < nfaces; ++i) {}
}

}