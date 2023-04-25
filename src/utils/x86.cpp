#include "utils/utils.hpp"
#include <bits/chrono.h>
#include <cassert>
#include <chrono>
#include <cpuid.h>
#include <cstdint>
#include <fcntl.h>
#include <ia32intrin.h>
#include <unistd.h>

namespace CodeBase {


// uint64_t GetCPUHZ() {
//     uint64_t tsc_hz = 0;
//     uint32_t a, b, c, d, maxleaf;
//     uint8_t  mult, model;
//     int32_t  ret;

//     /*
// 	 * Time Stamp Counter and Nominal Core Crystal Clock
// 	 * Information Leaf
// 	 */
//     maxleaf = __get_cpuid_max(0, NULL);
//     assert(maxleaf >= 0x15);

//     __cpuid(0x15, a, b, c, d);

//     /* EBX : TSC/Crystal ratio, ECX : Crystal Hz */
//     assert(b && c);
//     return c * (b / a);
// }


// thread_local uint64_t BeginTime =
//         std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
//                 .count();
// static uint64_t CycleHZ = GetCPUHZ();

// uint64_t now() {
//     unsigned int cpuid = 0;

//     auto cycleCount = __builtin_ia32_rdtscp(&cpuid);

//     return cycleCount / CycleHZ - BeginTime;
// }

uint64_t now() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
}
}// namespace CodeBase