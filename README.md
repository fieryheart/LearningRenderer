# LearningRenderer

# GouraudShader
```c++
class GouraudShader : public Shader {  
public:
    Vec3f varying_intensity;
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Vec3f light;

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;    
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light, 0.f, 1.f);
        Vec4f gl_vertex = model->vert(nthface, nthvert);
        gl_vertex = mat_transform*gl_vertex;
        return  gl_vertex;
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};
```

# Phong Shading
```c++
// no shadow
class PhongShader : public Shader {
public:
    // 变换矩阵
    Matrix mat_model, mat_view, mat_project, mat_viewport;
    Matrix mat_transform;
    Matrix mat_normal, mat_normal_it;
    Vec3f light;

    Vec4f gl_vertex[3];
    Vec3f tex_uv[3];

    void computeTranform() {
        mat_transform = mat_viewport*mat_project*mat_view*mat_model;
        mat_normal = mat_project*mat_view*mat_model;
        mat_normal_it = mat_normal.inverse().transpose();
    }

    virtual Vec4f vertex(int nthface, int nthvert) {
        // varying_intensity[nthvert] = std::clamp(model->norm(nthface, nthvert)*light1_dir, 0.f, 1.f);
        gl_vertex[nthvert] = model->vert(nthface, nthvert);
        gl_vertex[nthvert] = mat_transform*gl_vertex[nthvert];
        tex_uv[nthvert] = model->texCoord(nthface, nthvert);
        return  gl_vertex[nthvert]/gl_vertex[nthvert][3];
    }
    virtual bool fragment(Vec3f bar, TGAColor &color) {
        // float intensity = varying_intensity*bar;
        Vec3f uv = tex_uv[0]*bar[0]+tex_uv[1]*bar[1]+tex_uv[2]*bar[2];
        Vec4f n(model->normal(uv),0.f);
        n = (mat_normal_it*n).normalize();
        Vec4f l(light, 0.f);
        l = (mat_normal*l).normalize();

        Vec4f r = (n*(n*l*2.f) - l).normalize();   // reflected light

        float spec = std::pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*l);

        TGAColor c = model->diffuse(uv);
        
        for (int i=0; i<3; i++) {
            // 5. : 环境光部分
            // 1. : 漫反射部分
            // .6 : 高光部分
            color[i] = std::min<float>(5. + c[i]*(1.*diff + .6*spec), 255);
        }

        return false;
    }
};
```



参考链接：https://github.com/ssloy/tinyrenderer/tree/d0703acf18c48f2f7d00e552697d4797e0669ade