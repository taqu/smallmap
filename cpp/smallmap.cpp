#include "smallmap.hpp"

namespace sm
{
namespace
{
uint32_t* lower_bound(uint32_t* first, uint32_t* last, const uint32_t& val)
{
    uintptr_t count = last - first;
    while(0 < count) {
        uintptr_t d = count / 2;
        uint32_t* m = first + d;
        if(*m < val) {
            first = m + 1;
            count -= d + 1;
        } else {
            count = d;
        }
    }
    return first;
}

uint32_t next_prime(uint32_t x)
{
    // clang-format off
    static constexpr uint32_t table[] = {
        5UL, 11UL, 17UL, 29UL, 37UL,
        53UL, 67UL, 79UL, 97UL, 131UL,
        193UL, 257UL, 389UL, 521UL, 769UL,
        1031UL, 1543UL, 2053UL, 3079UL, 6151UL,
        12289UL, 24593UL, 49157UL, 98317UL, 196613UL,
        393241UL, 786433UL, 1572869UL, 3145739UL, 6291469UL,
        12582917UL, 25165843UL, 50331653UL, 100663319UL, 201326611UL,
        402653189UL, 805306457UL, 1610612741UL, 3221225473UL, 4294967291UL,
    };
    static constexpr uint32_t size = sizeof(table)/sizeof(table[0]);
    // clang-format on

    const uint32_t* const prime_list_begin = table;
    const uint32_t* const prime_list_end = prime_list_begin + size;
    const uint32_t* bound = lower_bound(prime_list_begin, prime_list_end, x);
    if(bound == prime_list_end) {
        --bound;
    }
    return *bound;
}
}
}

