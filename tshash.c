/* The Unlicense
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.
In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <http://unlicense.org/>
*/
#include "tshash.h"

#ifdef _MSC_VER
#include <intrin.h>
#define TSHASH_RESTRICT(x) x __restrict
#else
#define TSHASH_RESTRICT(x) x __restrict__
#endif

#ifdef __cplusplus
#include <cassert>
#include <cstring>

#if 202002L<=__cpluspus
#include <bit>
#endif

#define TSHASH_STATIC
#define TSHASH_NAMESPACE_BEGIN namespace {
#define TSHASH_NAMESPACE_END }
using TSHASH_NULL = nullptr;

#else
#include <assert.h>
#include <string.h>
#define TSHASH_NULL NULL
#define TSHASH_STATIC static
#define TSHASH_NAMESPACE_BEGIN
#define TSHASH_NAMESPACE_END

#endif

TSHASH_NAMESPACE_BEGIN
static const tshash_u64 Seeds[] = {
    0xB2D14DF0743669B1ULL,
    0xA6553A9CA35693C5ULL,
    0x6A2B6696D1B8D8D1ULL,
    0xC35A93A3D287A5D1ULL,
    0xB18D0F8759D13363ULL,
};
TSHASH_NAMESPACE_END

#if defined(_MSC_VER)
#define TSHASH_SWAPBYTES32(x) \
    _byteswap_ulong(x)

#define TSHASH_SWAPBYTES64(x) \
    _byteswap_uint64(x)

#elif defined(__GNUC__) || defined(__clang__)
#define TSHASH_SWAPBYTES32(x) \
    __builtin_bswap32(x)

#define TSHASH_SWAPBYTES64(x) \
    __builtin_bswap64(x)

#else

TSHASH_EXTERN_C
TSHASH_NAMESPACE_BEGIN
TSHASH_STATIC tshash_u32 TSHASH_SWAPBYTES32(tshash_u32 x)
{
    tshash_u32 result = (x & 0xFF000000UL) >> 24ULL;
    result |= (x & 0x00FF0000UL) >> 8ULL;
    result |= (x & 0x0000FF00UL) << 8ULL;
    result |= (x & 0x000000FFUL) << 24ULL;
    return result;
}

TSHASH_STATIC tshash_u64 TSHASH_SWAPBYTES64(tshash_u64 x)
{
    tshash_u64 result = (x & 0xFF00000000000000ULL) >> 56ULL;
    result |= (x & 0x00FF000000000000ULL) >> 40ULL;
    result |= (x & 0x0000FF0000000000ULL) >> 24ULL;
    result |= (x & 0x000000FF00000000ULL) >> 8ULL;
    result |= (x & 0x00000000FF000000ULL) << 8ULL;
    result |= (x & 0x0000000000FF0000ULL) << 24ULL;
    result |= (x & 0x000000000000FF00ULL) << 40ULL;
    result |= (x & 0x00000000000000FFULL) << 56ULL;
    return result;
}
TSHASH_NAMESPACE_END
TSHASH_EXTERN_C_END
#endif

#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define TSHASH_CORRECT_ENDIAN32(x) (x)
#define TSHASH_CORRECT_ENDIAN64(x) (x)
#else
#define TSHASH_CORRECT_ENDIAN32(x) TSHASH_SWAPBYTES32(x)
#define TSHASH_CORRECT_ENDIAN64(x) TSHASH_SWAPBYTES64(x)
#endif

TSHASH_EXTERN_C
TSHASH_NAMESPACE_BEGIN
TSHASH_STATIC tshash_u64 tshash_load32(const tshash_u8* const ptr)
{
    tshash_u32 result;
    memcpy(&result, ptr, sizeof(tshash_u32));
    return TSHASH_CORRECT_ENDIAN32(result);
}

TSHASH_STATIC tshash_u64 tshash_load64(const tshash_u8* const ptr)
{
    tshash_u64 result;
    memcpy(&result, ptr, sizeof(tshash_u64));
    return TSHASH_CORRECT_ENDIAN64(result);
}

TSHASH_STATIC tshash_u64 tshash_load12(const tshash_u8* const ptr, size_t l)
{
    tshash_u64 x0 = ptr[0];
    tshash_u64 x1 = ptr[l>>1];
    tshash_u64 x2 = ptr[l-1];
    return (x0<<16) | (x1<<8) | x2;
}

TSHASH_NAMESPACE_END

#if defined(__SIZEOF_INT128__) && !(TSHASH_DEBUG_NO_INTRINSIC)
TSHASH_NAMESPACE_BEGIN
inline TSHASH_STATIC tshash_u64 tshash_mum(tshash_u64 x0, tshash_u64 x1)
{
    __uint128_t r = ((__uint128_t)x0) * ((__uint128_t)x1);
    x0 ^= (tshash_u64)r;
    x1 ^= (tshash_u64)(r>>64);
    return x0^x1;
}
TSHASH_NAMESPACE_END

#elif defined(_MSC_VER) && defined(_M_X64) && !(TSHASH_DEBUG_NO_INTRINSIC)
TSHASH_NAMESPACE_BEGIN
inline TSHASH_STATIC tshash_u64 tshash_mum(tshash_u64 x0, tshash_u64 x1)
{
    uint64_t t0, t1;
    t0 = _umul128(x0, x1, &t1);
    x0 ^= t0;
    x1 ^= t1;
    return x0^x1;
}
TSHASH_NAMESPACE_END

#else
TSHASH_NAMESPACE_BEGIN
TSHASH_STATIC tshash_u64 tshash_mum(tshash_u64 x0, tshash_u64 x1)
{
    tshash_u64 l0 = (tshash_u32)x0;
    tshash_u64 h0 = x0>>32;
    tshash_u64 l1 = (tshash_u32)x1;
    tshash_u64 h1 = x1>>32;

    tshash_u64 h0l1 = h0*l1;
    tshash_u64 h0h1 = h0*h1;
    tshash_u64 l0l1 = l0*l1;
    tshash_u64 l0h1 = l0*h1;
    tshash_u64 t = l0l1 + (h0l1<<32);
    tshash_u64 carry = t<l0l1;
    tshash_u64 tl = t + (l0h1<<32);
    carry += tl<t;
    tshash_u64 th = h0h1 + (h0l1>>32) + (l0h1>>32) + carry;
    x0 ^= tl;
    x1 ^= tl;
    return x0^x1;
}
TSHASH_NAMESPACE_END
#endif

tshash_u64 tshash64(size_t size, const void* const data, tshash_u64 seed)
{
    assert((0<size && TSHASH_NULL != data) || (size<=0));
    const tshash_u8* key = (const uint8_t*)data;
    seed ^= Seeds[0];
    tshash_u64 a,b;
    if(size<=16){
        if(4<=size){
            size_t remain = (size>>3)<<2;
            a = (tshash_load32(key)<<32) | tshash_load32(key+remain);
            b = (tshash_load32(key+size-4)<<32) | tshash_load32(key+size-4-remain);
        }else if(0<size){
            a = tshash_load12(key, size);
            b = 0;
        }else{
            a = b = 0;
        }
    }else{
        uint64_t l = size;
        if(48<l){
            tshash_u64 seed0 = seed;
            tshash_u64 seed1 = seed;
            do{
                seed = tshash_mum(tshash_load64(key)^Seeds[1], tshash_load64(key+8)^seed);
                seed0 = tshash_mum(tshash_load64(key+16)^Seeds[2], tshash_load64(key+24)^seed0);
                seed1 = tshash_mum(tshash_load64(key+32)^Seeds[3], tshash_load64(key+40)^seed1);
                key += 48;
                l -= 48;
            }while(48<l);
            seed ^= seed0^seed1;
        }
        while(16<l){
            seed = tshash_mum(tshash_load64(key)^Seeds[1], tshash_load64(key+8)^seed);
            key += 16;
            l -= 16;
        }
        a = tshash_load64(key+l-16);
        b = tshash_load64(key+l-8);
    }
    return tshash_mum(Seeds[4]^size, tshash_mum(a^Seeds[1], b^seed));
}

tshash_u32 tshash32(size_t size, const void* const data, tshash_u64 seed)
{
    tshash_u64 hash = tshash64(size, data, seed);
    return (tshash_u32)((hash>>32ULL) ^ (hash&0xFFFFFFFFULL));
}
TSHASH_EXTERN_C_END
