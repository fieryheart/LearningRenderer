#include "rasterizer.h"

namespace QGL {

int NUM_THREADS = 8;

Matrix MAT_MODEL = Matrix::identity(4);
Matrix MAT_VIEW = Matrix::identity(4);
Matrix MAT_PERS_PROJECT = Matrix::identity(4);
Matrix MAT_ORTHO_PROJECT = Matrix::identity(4);
Matrix MAT_SCREEN = Matrix::identity(4);     // 屏幕空间
Matrix MAT_TRANS = Matrix::identity(4);

void LookAt(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();
    Matrix viewMatrix = Matrix::identity(4);
    Matrix mat = Matrix::identity(4);
    for (int i = 0; i < 3; ++i) {
        viewMatrix[0][i] = x[i];
        viewMatrix[1][i] = y[i];
        viewMatrix[2][i] = z[i];
        mat[i][3] = -center[i];
    }
    MAT_VIEW = viewMatrix*mat;
}

void SetPerspectiveProjectMat(float near, float far) {
    MAT_PERS_PROJECT = Matrix::identity(4);
    MAT_PERS_PROJECT[0][0] = near;
    MAT_PERS_PROJECT[1][1] = near;
    MAT_PERS_PROJECT[2][2] = near+far;
    MAT_PERS_PROJECT[2][3] = -near*far;
    MAT_PERS_PROJECT[3][2] = 1;
}

void SetOrthogonalProjectMat(int l, int r, int b, int t, int n, int f) {
    MAT_ORTHO_PROJECT = Matrix::identity(4);
    Matrix scale = Matrix::identity(4);
    scale[0][0] = 2.0f/(r-l);
    scale[1][1] = 2.0f/(t-b);
    scale[2][2] = 2.0f/(n-f);

    Matrix traslate = Matrix::identity(4);
    traslate[0][3] = -(r+l)/2.0f;
    traslate[1][3] = -(t+b)/2.0f;
    traslate[2][3] = -(n+f)/2.0f;

    MAT_ORTHO_PROJECT = scale*traslate*MAT_ORTHO_PROJECT;    
}
void SetOrthogonalProjectMat(int width, int height, int depth) {
    MAT_ORTHO_PROJECT = Matrix::identity(4);
    Matrix scale = Matrix::identity(4);
    scale[0][0] = 2.0f/width;
    scale[1][1] = 2.0f/height;
    scale[2][2] = 2.0f/depth;
    Matrix traslate = Matrix::identity(4);
    MAT_ORTHO_PROJECT = scale*traslate*MAT_ORTHO_PROJECT;
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
    MAT_TRANS = MAT_SCREEN*MAT_ORTHO_PROJECT*MAT_PERS_PROJECT*MAT_VIEW*MAT_MODEL;
}

}