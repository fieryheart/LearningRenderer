#include "transform.hpp"

namespace QGL {

Matrix GenScaleMat(Vec3f s) {
    Matrix mat = Matrix::identity(4);
    mat[0][0] = s.x;
    mat[1][1] = s.y;
    mat[2][2] = s.z;
    return mat;
}

Matrix GenRotateMat(Vec3f r) {
    Matrix mat = Matrix::identity(4);
    // 
    return mat;
}

Matrix GenTranslateMat(Vec3f t) {
    Matrix mat = Matrix::identity(4);
    mat[0][3] = t.x;
    mat[1][3] = t.y;
    mat[2][3] = t.z;
    return mat;
}

Matrix GenModelMat(Vec3f s, Vec3f r, Vec3f t) {
    return GenTranslateMat(t)*GenRotateMat(r)*GenScaleMat(s);
}

Matrix GenViewMat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (cross(up,z)).normalize();
    Vec3f y = (cross(z,x)).normalize();
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    std::cout << z << std::endl;
    Matrix mat0 = Matrix::identity(4);
    Matrix mat1 = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        mat0[0][i] = x[i];
        mat0[1][i] = y[i];
        mat0[2][i] = z[i];
        mat1[i][3] = -eye[i];
    }
    return mat0*mat1;
}

Matrix GenOrthoMat(float l, float r, float t, float b, float n, float f) {
    Matrix mat0 = Matrix::identity(4);
    Matrix mat1 = Matrix::identity(4);

    mat0[0][0] = 2/(r-l);
    mat0[1][1] = 2/(t-b);
    mat0[2][2] = 2/(n-f);

    mat1[0][3] = -(r+l)/2;
    mat1[1][3] = -(t+b)/2;
    mat1[2][3] = -(n+f)/2;

    return mat0*mat1;
}

// 
Matrix GenPerspMat(float fov, float ratio, float near, float far) {
    Matrix mat = Matrix::identity(4);
    float d = 1/tan((fov/2)*M_PI/180);
    float A = -(near+far)/(far-near);
    float B = -2*far*near/(far-near);
    mat[0][0] = d/ratio;
    mat[1][1] = d;
    mat[2][2] = A;
    mat[2][3] = B;
    mat[3][2] = -1;
    return mat;
}

Matrix GenViewportMat(int x, int y, int w, int h, int depth) {
    Matrix mat = Matrix::identity(4);
    mat[0][0] = w/2.f;
    mat[1][1] = h/2.f;
    mat[2][2] = depth/2.f;

    mat[0][3] = x+w/2.f;
    mat[1][3] = y+h/2.f;
    mat[2][3] = depth/2.f;
    return mat;
}

}