#include <print>
#include <cmath>

#include "generation/perlin_noise.hpp"

static constexpr float INV_SQRT_2 = 0.70710678118f;

static float fade(float t) {
    return t * t * t* (t * (t * 6 - 15) + 10);
}

static float grad(int hash, float x, float y) {
    switch (hash & 0x7) {
    case 0x0: return  x;
    case 0x1: return -x;
    case 0x2: return  y;
    case 0x3: return -y;
    case 0x4: return ( x + y) * INV_SQRT_2;
    case 0x5: return (-x + y) * INV_SQRT_2;
    case 0x6: return ( x - y) * INV_SQRT_2;
    case 0x7: return (-x - y) * INV_SQRT_2;
    default: return 0;
    }
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static int inc(int num, int repeat) {
    num++;

    if (repeat > 0)
        num %= repeat;

    return num;
}

static int p[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
    36,103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234,
    75, 0,26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237,
    149, 56,87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48,
    27, 166,77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105,
    92, 41, 55,46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73,
    209, 76, 132,187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109,198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38,
    147, 118, 126,255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189,
    28, 42, 223, 183,170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153,
    101, 155, 167, 43, 172,9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
    232, 178, 185, 112, 104,218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144,
    12, 191, 179, 162, 241, 81,51, 145, 235, 249, 14, 239, 107, 49, 192, 214,
    31, 181, 199, 106, 157, 184, 84,204, 176, 115, 121, 50, 45, 127, 4, 150,
    254, 138, 236, 205, 93, 222, 114, 67,29, 24, 72, 243, 141, 128, 195, 78, 66,
    215, 61, 156, 180

    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
    36,103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234,
    75, 0,26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237,
    149, 56,87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48,
    27, 166,77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105,
    92, 41, 55,46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73,
    209, 76, 132,187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109,198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38,
    147, 118, 126,255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189,
    28, 42, 223, 183,170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153,
    101, 155, 167, 43, 172,9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
    232, 178, 185, 112, 104,218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144,
    12, 191, 179, 162, 241, 81,51, 145, 235, 249, 14, 239, 107, 49, 192, 214,
    31, 181, 199, 106, 157, 184, 84,204, 176, 115, 121, 50, 45, 127, 4, 150,
    254, 138, 236, 205, 93, 222, 114, 67,29, 24, 72, 243, 141, 128, 195, 78, 66,
    215, 61, 156, 180
};

namespace generation {
    void test_generation() {
        std::println("gen test");
    }

    float perlin2D(float x, float y, int repeat) {
        // calculate local coordinates (local tile / square)
        if (repeat > 0) {
            x = std::fmod(x, static_cast<float>(repeat));
            if (x < 0.0f)
                x += repeat;

            y = std::fmod(y, static_cast<float>(repeat));
            if (y < 0.0f)
                y += repeat;
        }

        const int x0 = static_cast<int>(std::floor(x));
        const int y0 = static_cast<int>(std::floor(y));

        const int xi = x0 & 255;
        const int yi = y0 & 255;

        const float xf = x - x0;
        const float yf = y - y0;

        // get u, v which are faded xf, yf variables
        const float u = fade(xf);
        const float v = fade(yf);

        // get gradient vectors for all vertices of the square
        const int aa = p[p[xi] + yi];
        const int ab = p[p[xi] + inc(yi, repeat)];
        const int ba = p[p[inc(xi, repeat)] + yi];
        const int bb = p[p[inc(xi, repeat)] + inc(yi, repeat)];

        // calculate dot product for each vertex of the square
        // and use bilinear interpolation to get final result withing [-1, 1]
        const float x1 = lerp(grad(aa, xf, yf),
                              grad(ab, xf - 1, yf),
                              u);

        const float x2 = lerp(grad(ba, xf, yf - 1),
                              grad(bb, xf - 1, yf - 1),
                              u);

        return lerp(x1, x2, v);
    }

    float octave_perlin2D(float x, float y, int octaves, float persistence,
                          int repeat)
    {
        float total {0.0};
        float frequency {1.0};
        float amplitude {1.0};
        float max_value {0.0};

        for (int i {0}; i < octaves; ++i) {
            total += perlin2D(x * frequency, y * frequency, repeat) * amplitude;

            max_value += amplitude;

            amplitude *= persistence;
            frequency *= 2;
        }

        return total / max_value;
    }
}
