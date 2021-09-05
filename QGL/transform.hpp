#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "../Math/math_init.hpp"

namespace QGL {

// 类OpenGL

Matrix GenScaleMat(Vec3f s);  // 放大缩小 矩阵

Matrix GenRotateXMat();  // 
Matrix GenRotateYMat();  // 
Matrix GenRotateZMat();  // 
Matrix GenRotateMat(Vec3f r);  // 旋转 矩阵

Matrix GenTranslateMat(Vec3f t);

// Model 矩阵
Matrix GenModelMat(Vec3f s, Vec3f r, Vec3f t);

// View 矩阵
Matrix GenViewMat(Vec3f eye, Vec3f center, Vec3f up);

// Orthogonal 矩阵
Matrix GenOrthoMat(float l, float r, float t, float b, float n, float f);

// Perspective 矩阵
Matrix GenPerspMat(float fov, float ratio, float near, float far);

Matrix GenViewportMat(int x, int y, int w, int h, int depth);
}
#endif