//g++ -std=c++17 -O2 -Iinclude hw6_constexpr_staticassert.cpp -o /tmp/w6_constexpr_staticassert && /tmp/w6_constexpr_staticassert

#include <iostream>
#include <array>
#include <cmath>
#include <cstddef>

// 1. Define the mathematical function to precompute
// Must be marked `constexpr` so it can execute during compilation.
constexpr double compute_value(size_t index) {
    // Example: Computing a normalized scaling function / sine wave approximation or reciprocal
    // std::sin became constexpr in C++26, so for portable C++17/20 we can use Taylor series or custom math.
    double x = static_cast<double>(index) * 0.01;
    // Example computation: Polynomial Taylor expansion for sin(x)
    double sin_x = x - (x*x*x) / 6.0 + (x*x*x*x*x) / 120.0 - (x*x*x*x*x*x*x) / 5040.0 + (x*x*x*x*x*x*x*x*x) / 362880.0 - (x*x*x*x*x*x*x*x*x*x*x) / 39916800.0 + (x*x*x*x*x*x*x*x*x*x*x*x*x) / 6227020800.0 - (x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x) / 1307674368000.0 + (x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x) / 355687428096000.0 - (x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x) / 121645100408832000.0 + (x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x) / 51090942171709440000.0;
    return sin_x;
}

// 2. Generator function to construct the lookup table array at compile time
template <typename T, size_t Size, typename Generator>
constexpr std::array<T, Size> generate_lookup_table(Generator gen) {
    std::array<T, Size> lut{};
    for (size_t i = 0; i < Size; ++i) {
        lut[i] = gen(i);
    }
    return lut;
}

// 3. Alternatively, a compile-time struct or array generator
template <size_t N>
struct SineLookupTable {
    std::array<double, N> values{};

    constexpr SineLookupTable() {
        for (size_t i = 0; i < N; ++i) {
            values[i] = compute_value(i);
        }
    }
};

// Compile-time instantiation using `inline constexpr` (stored in read-only .rodata segment)
constexpr size_t TABLE_SIZE = 1000;
inline constexpr auto SINE_LUT = generate_lookup_table<double, TABLE_SIZE>(compute_value);

// 4. Verification at Compile Time using static_assert
static_assert(SINE_LUT.size() == 1000, "Lookup table size mismatch!");
static_assert(SINE_LUT[0] == 0.0, "Compile-time calculation check failed for index 0");

int main() {
    std::cout << "=== Compile-Time Generated Lookup Table ===" << std::endl;

    // Runtime lookups are O(1) array index access with zero arithmetic overhead
    size_t runtime_indices[] = {0, 50, 100, 200, 300, 400};

    for (size_t idx : runtime_indices) {
        std::cout << "LUT[" << idx << "] = " << SINE_LUT[idx] << std::endl;
    }

    return 0;
}