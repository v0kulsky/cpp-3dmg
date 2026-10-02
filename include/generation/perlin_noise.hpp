#pragma once

#include <random>
#include <cstdint>
#include <array>

namespace generation {

class PerlinNoiseGenerator {
public:
    PerlinNoiseGenerator();
    PerlinNoiseGenerator(std::uint32_t seed,
                         std::uint32_t repeat = 0);

    void set_seed(std::uint32_t seed);
    std::uint32_t get_seed() const;

    float perlin2D(float x, float y) const;
    float octave_perlin2D(float x, float y,int octaves,
                          float persistence) const;

    void set_repeat(std::uint32_t repeat);
    std::uint32_t get_repeat() const;

private:
    void initialize_perm_array();
    float grad(int hash, float x, float y) const;
    int inc(int num) const;

    static constexpr std::uint32_t k_perm_array_size {512};

    std::array<std::uint16_t, k_perm_array_size> m_perm_array;
    std::uint32_t m_seed;
    std::uint32_t m_repeat;
};

}
