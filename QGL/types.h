#ifndef __TYPES_H__
#define __TYPES_H__

namespace QGL {

// 计算方式
enum ComType {
    CT_Single = 0,
    CT_Multi
};

// 贴图类型
enum MapType {
    MT_Diffuse = 0,
    MT_Normal,
    MT_Specular
};

// 模型类型
enum ModelType {
	MDT_Buildin = 0,
    MDT_Strange,
    MDT_Light
};

//
enum MatrialType {
    MTT_Diffuse = 0,
    MTT_Mirror,
    MTT_Glossy,
    MTT_Transparent
};
}


#endif // __TYPES_H__