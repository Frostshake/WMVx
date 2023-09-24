#include "../../stdafx.h"
#include "Math.h"

namespace core {

    float Random::normalised()
    {
        return std::rand() / static_cast<float>(RAND_MAX);
    }

    float Random::between(float lower, float upper)
    {
        return lower + (upper - lower) * (std::rand() / static_cast<float>(RAND_MAX));
    }

    int32_t Random::between(int32_t lower, int32_t upper)
    {
        return lower + static_cast<int>((upper + 1 - lower) * Random::normalised());
    }

}