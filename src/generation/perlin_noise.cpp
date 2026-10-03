#include <print>
#include <cmath>
#include <algorithm>

#include "generation/perlin_noise.hpp"

static constexpr float INV_SQRT_2 = 0.70710678118f;

static float fade(float t) {
    return t * t * t* (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

namespace generation {

PerlinNoiseGenerator::PerlinNoiseGenerator()
    : PerlinNoiseGenerator(std::random_device{}(), 0)
{}

PerlinNoiseGenerator::PerlinNoiseGenerator(unsigned int seed, std::uint32_t repeat)
    : m_seed(seed), m_repeat(repeat), m_gen(m_seed)
{
    prepare_perm_array();
}

void PerlinNoiseGenerator::set_seed(unsigned int seed) {
    m_seed = seed;
    m_gen.seed(m_seed);

    prepare_perm_array();
}

void PerlinNoiseGenerator::set_random_seed() {
    set_seed(std::random_device{}());
}

unsigned int PerlinNoiseGenerator::get_seed() const {
    return m_seed;
}

float PerlinNoiseGenerator::perlin2D(float x, float y) const {
    // calculate local coordinates (local tile / square)
    if (m_repeat > 0) {
        x = std::fmod(x, static_cast<float>(m_repeat));
        if (x < 0.0f)
            x += m_repeat;

        y = std::fmod(y, static_cast<float>(m_repeat));
        if (y < 0.0f)
            y += m_repeat;
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
    const int aa = m_perm_array[m_perm_array[xi] + yi];
    const int ab = m_perm_array[m_perm_array[xi] + inc(yi)];
    const int ba = m_perm_array[m_perm_array[inc(xi)] + yi];
    const int bb = m_perm_array[m_perm_array[inc(xi)] + inc(yi)];

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

float PerlinNoiseGenerator::octave_perlin2D(float x, float y, int octaves,
                                            float persistence) const
{
    float total {0.0};
    float frequency {1.0};
    float amplitude {1.0};
    float max_value {0.0};

    for (int i {0}; i < octaves; ++i) {
        total += perlin2D(x * frequency, y * frequency) * amplitude;

        max_value += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    return total / max_value;
}

void PerlinNoiseGenerator::set_repeat(std::uint32_t repeat) {
    m_repeat = repeat;
}

std::uint32_t PerlinNoiseGenerator::get_repeat() const {
    return m_repeat;
}

void PerlinNoiseGenerator::prepare_perm_array() {
    if (m_first_init) {
        std::iota(m_perm_array.begin(), m_perm_array.begin() + 256, 0);

        m_first_init = false;
    }

    std::shuffle(m_perm_array.begin(), m_perm_array.begin() + 256, m_gen);

    std::copy(m_perm_array.begin(), m_perm_array.begin() + 256,
              m_perm_array.begin() + 256);
}

float PerlinNoiseGenerator::grad(int hash, float x, float y) const {
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

int PerlinNoiseGenerator::inc(int num) const {
    num++;

    if (m_repeat > 0)
        num %= m_repeat;

    return num;
}

}
