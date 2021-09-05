#ifndef __SSAO_H__
#define __SSAO_H__

#include "pieces.hpp"

namespace QGL {

void ScreenSpaceAmbientOcclusion(Frame &frame, Zbuffer &zb);

}

#endif // __SSAO_H__