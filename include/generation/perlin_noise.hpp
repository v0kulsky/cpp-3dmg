#pragma once

namespace generation {
    void test_generation();

    float perlin(float x, float y);

    float octave_perlin2D(float x, float y, int octaves, float persistence,
                          int repeat = -1);
}
