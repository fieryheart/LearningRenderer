#ifndef __LAB_TEST_H__
#define __LAB_TEST_H__

#include <random>

void Lab_Test() {

    std::default_random_engine random(time(NULL));
    std::uniform_real_distribution<float> dist01(0.0, 1.0);
    std::uniform_real_distribution<float> dist11(-1.0, 1.0);

    for(int i = 0; i < 10; ++i)
        std::cout << dist11(random) << std::endl;
}

#endif