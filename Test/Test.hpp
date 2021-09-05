#ifndef __TEST_H__
#define __TEST_H__
#include <iostream>
#include "../Math/math_init.hpp"
using namespace std;
using namespace QGL;

void Test()
{

}

void TestVec2()
{
    // 构建
    Vec2f v0;
    cout << v0 << endl;

    Vec2f v1 = 1;
    cout << v1 << endl;

    Vec2f v2 = v1;

    Vec2f v3 = move(v2);

    // 加法
    Vec2f v4 = 1, v5 = 2;
    cout << (v4 + 1) << endl;
    cout << (v4 + v5) << endl;
    cout << (v4 + Vec2f(1, 2)) << endl;

    // 乘法
    Vec2f v6 = {2, 5}, v7 = {3, 4};
    cout << "v6 * 4: " << v6 * 4 << endl;
    cout << "v6 * v7: " << v6 * v7 << endl;
}

#endif