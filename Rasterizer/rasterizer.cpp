#include "rasterizer.hpp"

namespace QGL {


Vec3f barycentric(Vec4f *pts, Vec2f P) {
    Vec3f u = cross(Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x), Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
    if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void Rasterizer::render() {
    Timer timer = QGL::Timer();
    timer.update();
    switch(comType) {
        case CT_Single:
            single(); break;
        case CT_Multi:
            break;
    }
    timer.show();
}

void Rasterizer::single() {
    // std::cout << "single" << std::endl;
    switch(shadingType) {
        case ST_Forwad:
            forward(); break;
        case ST_Deferred:
            break;
    }
}

void Rasterizer::forward() {
    std::cout << "forward" << std::endl;
    int nfaces = model->nfaces();
    InVert in;
    OutVert out;
    in.model = model;
    // std::cout << nfaces << std::endl;
    for (int i = 0; i < nfaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vec3f vertex = model->vert(i, j);

            in.vertex = vertex;
            in.nthface = i;
            in.nthvert = j;
            
            shader->vertex(in, out);
            screen_vertexes[j] = out.vertex;
            // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
        }
        // std::cout << screen_vertexes[0] << std::endl;
        // std::cout << screen_vertexes[1] << std::endl;
        // std::cout << screen_vertexes[2] << std::endl;
        scan();
        if (log.flag) log.show(i, nfaces);
    }
    if (log.flag) log.show(nfaces, nfaces);  
}

void Rasterizer::scan() {
    // std::cout << "scan" << std::endl;
    int width = frame.width, height = frame.height;
    Vec4f *points = screen_vertexes;
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
    // std::cout << bboxmin << std::endl;
    // std::cout << bboxmax << std::endl;
    InFrag in;
    OutFrag out;
    in.model = model;
    in.lights = lights;
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
        for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
            bc = barycentric(points, Vec2f(P.x, P.y));
            z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
            w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
            z = 1-z;
            // z = z/w;
            // depth = std::max(0.f, std::min(1.0f, z/w));
            // std::cout << points[0] << std::endl;
            // std::cout << points[1] << std::endl;
            // std::cout << points[2] << std::endl;
            // std::cout << zbuffer.get(P.x, P.y) << " " << z << std::endl;
            // std::cout << bc << std::endl;
            if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer.get(P.x, P.y) < z) continue;
            in.bar = bc;
            in.depth = z;
            if (!shader->fragment(in, out)) {
                zbuffer.set(P.x, P.y, z);
                frame.set(P.x, P.y, out.color);
            }
        }
    }
}

void Rasterizer::shadow(Matrix model_mat) {

    for (auto &light : lights) {
        // light->GenShadowMap(width, height);
        Matrix world2viewport = Matrix::identity(4);
        if (light->type() == L_Direct) {
            Vec3f camera(100, 100, 100);
            Vec3f center = camera + light->Dir();
            Vec3f up(0,1,0);

            Matrix view_mat = GenViewMat(camera, center, up);

            Vec4f bboxmin(std::numeric_limits<float>::max());
            Vec4f bboxmax(-std::numeric_limits<float>::max());
            int width = frame.width, height = frame.height, depth = 1;
            // 根据对象生成aabb
            int nfaces = model->nfaces();
            for (int i = 0; i < nfaces; ++i) {
                for (int j = 0; j < 3; ++j) {
                    Vec4f vertex = Vec4f(model->vert(i, j), 1.0);
                    vertex = view_mat*model_mat*vertex;
                    for (int k = 0; k < 3; ++k) {
                        bboxmin[k] = fmin(bboxmin[k], vertex[k]);
                        bboxmax[k] = fmax(bboxmax[k], vertex[k]);
                    }
                }
            }
            bboxmin[3] = bboxmax[3] = 1.0f;

            Vec4f bboxmin2light = bboxmin-0.1;
            Vec4f bboxmax2light = bboxmax+0.1;
            std::cout << "bboxmin: " << bboxmin << std::endl;
            std::cout << "bboxmax: " << bboxmax << std::endl;

            float l = fmin(bboxmin2light[0], bboxmax2light[0]);
            float r = fmax(bboxmin2light[0], bboxmax2light[0]);
            float t = fmax(bboxmin2light[1], bboxmax2light[1]);
            float b = fmin(bboxmin2light[1], bboxmax2light[1]);
            float n = fmax(bboxmin2light[2], bboxmax2light[2]);
            float f = fmin(bboxmin2light[2], bboxmax2light[2]);

            // std::cout << l << " " << r << std::endl;
            // std::cout << b << " " << t << std::endl;
            // std::cout << n << " " << f << std::endl;

            Matrix ortho_mat = GenOrthoMat(l,r,t,b,n,f);

            Matrix viewport_mat = GenViewportMat(0, 0, width, height, depth);

            world2viewport = viewport_mat*ortho_mat*view_mat*model_mat;
            // world2viewport = ortho_mat*view_mat*model_mat;

            DirectLightDepthShader directLightDepthShader;
            directLightDepthShader.uniform_mat_transform = world2viewport;
            shader = &directLightDepthShader;

            // std::cout << model_mat << std::endl;
            // std::cout << view_mat << std::endl;
            // std::cout << ortho_mat << std::endl;
            // std::cout << viewport_mat << std::endl;

            forward();

            light->shadow = frame.copy();
            frame.clear();
            zbuffer.clear();
        }
    }
}

void Rasterizer::draw(std::string path, bool flip) {
    std::cout << "draw" << std::endl;
    Frame out = frame.copy();
    if (flip) out.flip();
    out.draw(path.c_str());
}
}

// int NUM_THREADS = 8;

// Matrix MAT_MODEL = Matrix::identity(4);
// Matrix MAT_VIEW = Matrix::identity(4);
// Matrix MAT_PROJECT = Matrix::identity(4);
// Matrix MAT_ORTHO_PROJECT = Matrix::identity(4);
// Matrix MAT_SCREEN = Matrix::identity(4);     // 屏幕空间
// Matrix MAT_TRANS = Matrix::identity(4);
// Matrix MAT_NORM_TRANS = Matrix::identity(4);

// void LookAt(Vec3f eye, Vec3f center, Vec3f up) {
//     Vec3f z = (eye - center).normalize();
//     Vec3f x = (cross(up,z)).normalize();
//     Vec3f y = (cross(z,x)).normalize();
//     std::cout << x;
//     std::cout << y;
//     std::cout << z;
//     Matrix viewMatrix = Matrix::identity(4);
//     Matrix mat = Matrix::identity(4);
//     for (int i = 0; i < 3; ++i) {
//         viewMatrix[0][i] = x[i];
//         viewMatrix[1][i] = y[i];
//         viewMatrix[2][i] = z[i];
//         mat[i][3] = -eye[i];
//     }
//     MAT_VIEW = viewMatrix*mat;
// }

// void SetProjectMat(float fov, float ratio, float near, float far) {
//     MAT_PROJECT = Matrix::identity(4);
//     float d = 1/tan((fov/2)*M_PI/180);
//     float A = -(near+far)/(far-near);
//     float B = -2*far*near/(far-near);
//     MAT_PROJECT[0][0] = d/ratio;
//     MAT_PROJECT[1][1] = d;
//     MAT_PROJECT[2][2] = A;
//     MAT_PROJECT[2][3] = B;
//     MAT_PROJECT[3][2] = -1;
// }

// // 计算屏幕坐标
// // x: [-1,1] -> [x,x+w]
// // y: [-1,1] -> [y,y+h]
// // z: [-1,1] -> [0,depth]
// void SetScreenMat(int x, int y, int w, int h, int depth) {
//     MAT_SCREEN = Matrix::identity(4);
//     MAT_SCREEN[0][0] = w/2.f;
//     MAT_SCREEN[1][1] = h/2.f;
//     MAT_SCREEN[2][2] = depth/2.f;

//     MAT_SCREEN[0][3] = x+w/2.f;
//     MAT_SCREEN[1][3] = y+h/2.f;
//     MAT_SCREEN[2][3] = depth/2.f;
// }

// void Init() {
//     MAT_TRANS = MAT_SCREEN*MAT_PROJECT*MAT_VIEW*MAT_MODEL;
//     MAT_NORM_TRANS = MAT_MODEL.inverse().transpose();
// }

// Vec3f barycentric(Vec4f *pts, Vec2f P) {
//     Vec3f u = cross(Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x), Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y));
//     if (std::abs(u.z)<1e-2) return Vec3f(-1,1,1);
//     return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
// }

// void Rasterize(RasterNode &rn) {
//     if (rn.comType == CT_Single) SingleRasterize(rn);
//     else if (rn.comType == CT_Multi) MultiRasterize(rn);
//     else return;
// }

// void DrawTriangle(Vec4f *points, RasterNode &rn) {
//     Model *model = rn.model;
//     Shader *shader = rn.shader;
//     Frame *frame = rn.frame;
//     Zbuffer *zbuffer = rn.zbuffer;
//     int width = frame->width, height = frame->height;

//     Vec2f bboxmin(width-1, height-1);
//     Vec2f bboxmax(0, 0);
//     Vec2f clamp(width-1, height-1); 
//     for (int i=0; i<3; i++) { 
//         bboxmin.x = std::min(bboxmin.x, points[i].x);
//         bboxmin.y = std::min(bboxmin.y, points[i].y);
//         bboxmax.x = std::max(bboxmax.x, points[i].x);
//         bboxmax.y = std::max(bboxmax.y, points[i].y);
//     }
//     bboxmin.x = std::max(0.f, bboxmin.x);
//     bboxmin.y = std::max(0.f, bboxmin.y);
//     bboxmax.x = std::min(clamp.x, bboxmax.x);
//     bboxmax.y = std::min(clamp.y, bboxmax.y);

//     Vec2i P;
//     Vec3f bc;
//     float z, w, depth, weight;
//     // std::cout << bboxmin;
//     // std::cout << bboxmax;
//     for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
//         for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
//             bc = barycentric(points, Vec2f(P.x, P.y));
//             z = points[0][2]*bc.x+points[1][2]*bc.y+points[2][2]*bc.z;
//             w = points[0][3]*bc.x+points[1][3]*bc.y+points[2][3]*bc.z;
//             // z = z/w;
//             // depth = std::max(0.f, std::min(1.0f, z/w));
//             if (bc.x < 0 || bc.y < 0 || bc.z < 0 || zbuffer->get(P.x, P.y) > z) continue;
//             InFrag in;
//             OutFrag out;
//             in.bar = bc;
//             in.depth = z;
//             in.model = model;
//             if (!shader->fragment(in, out)) {
//                 zbuffer->set(P.x, P.y, z);
//                 frame->set(P.x, P.y, out.color);
//             }
//         }
//     }
// }

// void SingleRasterize(RasterNode &rn) {
//     Model *model = rn.model;
//     Shader *shader = rn.shader;
//     Log *log = rn.log;
//     // Zbuffer *zbuffer = rn.zbuffer;
// 	Frame *frame = rn.frame;
//     Vec4f screen_coords[3];
//     int nfaces = model->nfaces();
//     for (int i = 0; i < nfaces; ++i) {
//         for (int j = 0; j < 3; ++j) {
//             Vec3f v = model->vert(i, j);
//             InVert inV;
//             OutVert outV;

//             inV.v = v;
//             inV.nthface = i;
//             inV.nthvert = j;
//             inV.model = model;
            
//             shader->vertex(inV, outV);
//             screen_coords[j] = outV.sCoord;
//             // std::cout << "Rendering-" << i << "-" << j << "-" << screen_coords[j];
//         }

//         DrawTriangle(screen_coords, rn);

//         if (log && log->flag) log->show(i, nfaces);
//     }
//     if (log && log->flag) log->show(nfaces, nfaces);
// }

// void MultiRasterize(RasterNode &rn) {
//     std::cout << "Multithreading rasterize is not supported~" << std::endl;

//     Model *model = rn.model;
//     int nfaces = model->nfaces();

//     #pragma omp parallel for num_threads(NUM_THREADS)
//     for (int i = 0; i < nfaces; ++i) {}
// }