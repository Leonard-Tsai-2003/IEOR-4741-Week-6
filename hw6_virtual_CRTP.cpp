//g++ -std=c++17 -O2 -Iinclude hw6_virtual_CRTP.cpp -o /tmp/w6_vbench && /tmp/w6_vbench

// hw4_crtpCallCost.cpp
// Port of HW 4 RiskCheck hierarchy to CRTP (Static Polymorphism)
#include <algorithm>
#include <cstdio>
#include <variant>
#include <vector>
#include "bench.hpp" 

// ============================================================================
// 1. Original HW 4 Virtual Hierarchy Baseline
// ============================================================================
struct RiskCheck { 
    virtual ~RiskCheck() = default; 
    virtual bool check(double price, int qty) = 0; 
}; 

struct R0 final : RiskCheck { bool check(double p, int q) override { return (p * q) <= 10000.0; } }; 
struct R1 final : RiskCheck { bool check(double p, int q) override { return q <= 100; } }; 
struct R2 final : RiskCheck { bool check(double p, int q) override { return p >= 10.0 && q > 0; } }; 
struct R3 final : RiskCheck { bool check(double p, int q) override { return (p * q) > 500.0; } }; 

struct DirectRisk { bool check(double p, int q) { return (p * q) <= 10000.0; } }; 

// ============================================================================
// 2. CRTP Base Class Template & Derived Rules
// ============================================================================
template <typename Derived>
struct RiskCheckCRTP {
    // Shared public interface executing static polymorphism
    bool check(double price, int qty) const {
        // Direct static dispatch to derived class implementation (zero virtual overhead)
        return static_cast<const Derived*>(this)->check_impl(price, qty);
    }
};

struct R0_CRTP : RiskCheckCRTP<R0_CRTP> {
    bool check_impl(double p, int q) const { return (p * q) <= 10000.0; }
};

struct R1_CRTP : RiskCheckCRTP<R1_CRTP> {
    bool check_impl(double p, int q) const { return q <= 100; }
};

struct R2_CRTP : RiskCheckCRTP<R2_CRTP> {
    bool check_impl(double p, int q) const { return p >= 10.0 && q > 0; }
};

struct R3_CRTP : RiskCheckCRTP<R3_CRTP> {
    bool check_impl(double p, int q) const { return (p * q) > 500.0; }
};

static double g_sink; // benchmark sink to prevent optimization

constexpr long N = 20'000'000; 

template <class F> double median_ns(F&& f, int reps = 7) { 
    std::vector<double> v; 
    for (int i = 0; i < reps; ++i) v.push_back(ns_per_op(f, N)); 
    std::sort(v.begin(), v.end()); 
    return v[reps / 2]; 
}

int main() {
    // Virtual Objects
    R0 r0; R1 r1; R2 r2; R3 r3; 
    RiskCheck* one = &r0; 
    RiskCheck* four[4] = {&r0, &r1, &r2, &r3}; 

    // CRTP Objects
    R0_CRTP crtp0; R1_CRTP crtp1; R2_CRTP crtp2; R3_CRTP crtp3;

    // Static heterogeneous container using std::tuple (Zero dynamic lookup overhead)
    auto crtp_tuple = std::make_tuple(crtp0, crtp1, crtp2, crtp3);

    // Pattern table: 4096 pseudo-random target indices
    std::vector<unsigned char> pat(4096); 
    unsigned x = 12345; 
    for (auto& p : pat) { x = x * 1664525u + 1014204223u; p = (x >> 24) & 3; } 

    DirectRisk d; 

    auto run = [&](auto&& body) { 
        double acc = 0.0; 
        long i = 0; 
        return median_ns([&] { 
            double price = 100.0 + (i & 7) * 0.1; 
            int qty = static_cast<int>(i & 127); 
            acc += body(price, qty, i) ? 1.0 : 0.0; 
            ++i; 
            doNotOptimize(acc); 
            g_sink = acc; 
        }); 
    };

    // --- HW 4 Virtual Benchmarks ---
    double t_direct = run([&](double p, int q, long)   { return d.check(p, q); }); 
    RiskCheck* volatile hide = one; 
    double t_virt1  = run([&](double p, int q, long)   { return hide->check(p, q); }); 
    double t_virt4r = run([&](double p, int q, long i) { return four[pat[i & 4095]]->check(p, q); }); 
    double t_virt4p = run([&](double p, int q, long i) { return four[i & 3]->check(p, q); }); 
    R0* volatile hide0 = &r0; 
    double t_final  = run([&](double p, int q, long)   { return hide0->check(p, q); }); 

    // --- CRTP Benchmarks ---
    // Single target direct CRTP (100% inlined)
    double t_crtp_direct = run([&](double p, int q, long) { return crtp0.check(p, q); });

    // Multi-target inlined CRTP via static tuple dispatch (No vtable, no std::visit)
    auto crtp_tuple_dispatch = [](const auto& tup, size_t idx, double p, int q) {
        switch (idx) {
            case 0: return std::get<0>(tup).check(p, q);
            case 1: return std::get<1>(tup).check(p, q);
            case 2: return std::get<2>(tup).check(p, q);
            case 3: return std::get<3>(tup).check(p, q);
        }
        return false;
    };

    double t_crtp_inlined_p = run([&](double p, int q, long i) {
        return crtp_tuple_dispatch(crtp_tuple, i & 3, p, q);
    });

    double t_crtp_inlined_r = run([&](double p, int q, long i) {
        return crtp_tuple_dispatch(crtp_tuple, pat[i & 4095], p, q);
    });

    std::printf("--- HW 4 Virtual Benchmarks ---\n");
    std::printf("direct call (no base class)          %6.2f ns/call\n", t_direct);
    std::printf("virtual, 1 target                    %6.2f ns/call\n", t_virt1);
    std::printf("virtual, 4 targets, i&3              %6.2f ns/call\n", t_virt4p);
    std::printf("virtual, 4 targets, random           %6.2f ns/call\n", t_virt4r);
    std::printf("final class, via R0*                 %6.2f ns/call\n", t_final);

    std::printf("\n--- CRTP Benchmarks ---\n");
    std::printf("CRTP direct call (R0_CRTP)           %6.2f ns/call\n", t_crtp_direct);
    std::printf("CRTP fully inlined tuple, i&3        %6.2f ns/call\n", t_crtp_inlined_p);
    std::printf("CRTP fully inlined tuple, random     %6.2f ns/call\n", t_crtp_inlined_r);
    std::printf("sink=%g\n", g_sink); 
}