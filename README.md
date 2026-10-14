# IEOR 4741 - Homework 6: CRTP, Compile-Time LUT, Policy Pattern, and static_assert

This repository contains the implementations and performance analysis for Homework 6, demonstrating modern C++ template metaprogramming techniques including the Curiously Recurring Template Pattern (CRTP), `constexpr` lookup tables, Policy-based design, and compile-time validation using `static_assert`.

---

## Summary of Tasks & Requirement Verification

### Task 1: CRTP Port of `RiskCheck` Hierarchy & Inlining Comparison

#### Why the Code Meets Requirements
- **Static Polymorphism**: The `RiskCheck` hierarchy from Homework 4 was refactored using CRTP (`template <typename Derived> class RiskCheck`). Static dispatch is achieved by casting `this` via `static_cast<const Derived*>(this)->check_impl(...)`, completely eliminating virtual functions and `vtable` lookups.
- **Assembly Inlining**: Compiling with `g++ -std=c++17 -O2 -S` demonstrates that the static calls are fully inlined into the calling loop rather than issuing indirect `call *vtable` instructions.
- **Benchmarking**: The CRTP implementation was benchmarked on the exact same machine and harness as Homework 4 to provide an accurate head-to-head comparison.

#### Execution & Benchmark Output
```bash
g++ -std=c++17 -O2 -Iinclude hw6_virtual_CRTP.cpp -o /tmp/w6_vbench && /tmp/w6_vbench
```

**Output**:
```text
--- HW 4 Virtual Benchmarks ---
direct call (no base class)            0.91 ns/call
virtual, 1 target                      1.08 ns/call
virtual, 4 targets, i&3                2.36 ns/call
virtual, 4 targets, random             6.76 ns/call
final class, via R0*                   0.89 ns/call

--- CRTP Benchmarks ---
CRTP direct call (R0_CRTP)             0.89 ns/call
CRTP fully inlined tuple, i&3          1.49 ns/call
CRTP fully inlined tuple, random       4.80 ns/call
sink=1.3475e+08
```

#### Comparison & Trade-off Analysis (What Was Gained vs. What Was Gave Up)
- **What Was Gained**:
  1. **Elimination of Virtual Dispatch Overhead**: Direct CRTP call latency (**0.89 ns/call**) matches raw direct calls and `final` classes, outperforming virtual dispatch (**1.08 ns/call**).
  2. **Superior Branching & Inlining Performance**: Under multi-target iteration via `std::tuple`, CRTP reduces call latency significantly compared to virtual calls (Bitwise pattern `i&3`: **1.49 ns** vs. **2.36 ns**; Random branch pattern: **4.80 ns** vs. **6.76 ns**). The compiler can unroll and inline the template calls, removing indirect branch mispredictions.
  3. **Zero Memory Overhead**: Removes the 8-byte virtual pointer (`vptr`) overhead per object instance.
- **What Was Gave Up**:
  1. **Dynamic / Heterogeneous Polymorphism**: CRTP creates distinct base types (`RiskCheck<DerivedA>` vs. `RiskCheck<DerivedB>`). You cannot store heterogeneous objects in a single runtime container (e.g., `std::vector<RiskCheck*>`).
  2. **Compile-Time Binding & Binary Bloat**: Strategies must be known at compile time and dispatched via constructs like `std::tuple` or `std::variant`. This increases template compilation time and potential code duplication (binary size bloat) due to monomorphization.

---

### Task 2: Compile-Time `constexpr` Lookup Table (LUT)

#### Why the Code Meets Requirements
- **Compile-Time Computation**: The lookup table generator function is declared `constexpr`, computing mathematical values (e.g., sine calculations) at compile time into a `std::array`.
- **Zero Runtime Overhead**: At runtime, accessing `LUT[i]` performs an $O(1)$ direct array index lookup with zero calculation or initialization overhead during runtime execution.

#### Execution Output
```bash
g++ -std=c++17 -O2 -Iinclude hw6_constexpr_staticassert.cpp -o /tmp/w6_constexpr_staticassert && /tmp/w6_constexpr_staticassert
```

**Output**:
```text
=== Compile-Time Generated Lookup Table ===
LUT[0] = 0
LUT[50] = 0.479426
LUT[100] = 0.841471
LUT[200] = 0.909297
LUT[300] = 0.141098
LUT[400] = -0.759272
```

---

### Task 3: Policy-Based Design Class

#### Why the Code Meets Requirements
- **Zero-Cost Behavior Selection**: Class behavior (e.g., report formatting) is parameterized via a policy template argument (`Reporter<Policy>`).
- **Compile-Time Swapping**: Switching policies (e.g., `PlainTextFormatter` vs. `MarkdownFormatter`) alters the output behavior completely at compile time without virtual function overhead or runtime pointer dereferencing.

#### Execution Output
```bash
g++ -std=c++17 -O2 -Iinclude hw6_policy.cpp -o /tmp/w6_policy && /tmp/w6_policy
```

**Output**:
```text
System Report
Status: Operational
---
# System Report

Status: Operational
```

---

### Task 4: Compile-Time Invariant Enforcement with `static_assert`

#### Why the Code Meets Requirements
- **Static Invariant Enforcement**: Embedded directly inside `hw6_constexpr_staticassert.cpp` (e.g., checking `static_assert(LUT.size() == 401, "Invalid LUT size");` or mathematical precision constraints).
- **Compile-Time Failure**: If an invariant condition evaluates to `false`, compilation immediately halts with a descriptive error message, preventing invalid states from entering runtime execution.

---

## How to Build and Run

Run the commands below to compile and run all benchmarks and tests:

```bash
# Compile and run CRTP vs Virtual Benchmarks
g++ -std=c++17 -O2 -Iinclude hw6_virtual_CRTP.cpp -o /tmp/w6_vbench && /tmp/w6_vbench

# Compile and run Constexpr LUT & static_assert
g++ -std=c++17 -O2 -Iinclude hw6_constexpr_staticassert.cpp -o /tmp/w6_constexpr_staticassert && /tmp/w6_constexpr_staticassert

# Compile and run Policy Design Demo
g++ -std=c++17 -O2 -Iinclude hw6_policy.cpp -o /tmp/w6_policy && /tmp/w6_policy
```