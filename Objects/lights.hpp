#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "../Math/vec.hpp"
#include "../QGL/pieces.hpp"
using namespace QGL;
class Light {
public:
    Frame shadow;
    Light() {}
    virtual ~Light() {}
    virtual Vec3f Dir() { return Vec3f(1.0, 0.0, 0.0); }
    virtual Vec3f Power() { return Vec3f(1.0, 1.0, 1.0); }
    virtual LightType type() { return L_default; }
    virtual void draw(std::string path, bool flip=true) {}
};

class DirectLight : public Light {
    Vec3f dir;
    Vec3f intensity;
    LightType LType;
public:
    DirectLight(Vec3f _dir, Vec3f _intensity=Vec3f(1.0, 1.0, 1.0)) : dir(_dir.normalize()), intensity(_intensity), LType(L_Direct) {}
    virtual Vec3f Dir() {
        return dir;
    }
    virtual Vec3f Power() {
        return intensity;
    }

    virtual LightType type() { return LType; }

    virtual void draw(std::string path, bool flip) {
        std::cout << "draw shadow" << std::endl;
        Frame out = shadow.copy();
        if (flip) out.flip();
        out.draw(path.c_str());
    }
};

class PointLight : public Light {
    Vec3f pos;
    Vec3f intensity;
    LightType LType;
public:
    PointLight(Vec3f _pos, Vec3f _intensity=Vec3f(1.0, 1.0, 1.0)) : pos(_pos), intensity(_intensity), LType(L_Point) {}
    Vec3f Dir(Vec3f sp) {
        return (sp-pos).normalize();
    }
    virtual Vec3f Power() {
        return intensity;
    }
    virtual LightType type() { return LType; }
};

#endif