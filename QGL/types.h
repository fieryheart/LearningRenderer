#ifndef __TYPES_H__
#define __TYPES_H__

namespace QGL {

// 计算方式
enum ComType {
    CT_Single = 0,
    CT_Multi
};

enum ShadingType {
    ST_Forwad = 0,
    ST_Deferred
};

// 贴图类型
enum MapType {
    MT_Diffuse = 0,
    MT_Normal,
    MT_Specular
};

// 对象类型
enum ObjectType {
	OBJ_Buildin = 0,
    OBJ_Strange
};

//
enum MatrialType {
    MTT_Diffuse = 0,
    MTT_Mirror,
    MTT_Glossy,
    MTT_Transparent
};

enum LightType {
    L_default = 0,
    L_Direct,
    L_Point
};

}


#endif // __TYPES_H__