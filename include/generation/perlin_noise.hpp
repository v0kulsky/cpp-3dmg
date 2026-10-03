#pragma once

#include <random>
#include <cstdint>
#include <array>
#include <filesystem>

namespace generation {

class PerlinNoiseGenerator {
public:
    PerlinNoiseGenerator();
    PerlinNoiseGenerator(std::uint32_t seed,
                         std::uint32_t repeat = 0);

    void set_seed(std::uint32_t seed);
    void set_random_seed();
    std::uint32_t get_seed() const;

    float perlin2D(float x, float y) const;
    float octave_perlin2D(float x, float y,int octaves,
                          float persistence) const;

    void set_repeat(std::uint32_t repeat);
    std::uint32_t get_repeat() const;

    bool save_to_pgm(const std::filesystem::path& path,
                     std::uint32_t width, std::uint32_t height,
                     float scale = 0.02f, int octaves = 1,
                     float persistence = 1.0f) const;

private:
    static constexpr std::uint32_t k_perm_array_size {512};

    std::uint32_t m_seed;
    std::uint32_t m_repeat;

    std::mt19937 m_gen;
    std::array<std::uint16_t, k_perm_array_size> m_perm_array;

    bool m_first_init {true};

    void prepare_perm_array();
    float grad(int hash, float x, float y) const;
    int inc(int num) const;
};

}
