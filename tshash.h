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
#ifndef INC_TSHASH_H_
#define INC_TSHASH_H_
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#include <stddef.h>
#endif

#ifdef __cplusplus
using tshash_u8 = uint8_t;
using tshash_u32 = uint32_t;
using tshash_u64 = uint64_t;

#define TSHASH_EXTERN_C extern "C" {
#define TSHASH_EXTERN_C_END }

static constexpr tshash_u64 TSHASH_DEFUALT_SEED = 0xD4A3D22E3C651BD1ULL;

#else
typedef uint8_t tshash_u8;
typedef uint32_t tshash_u32;
typedef uint64_t tshash_u64;

#define TSHASH_EXTERN_C
#define TSHASH_EXTERN_C_END

#define TSHASH_DEFUALT_SEED (0xD4A3D22E3C651BD1ULL)
#endif

TSHASH_EXTERN_C
/**
 @brief Calculate the hash of data.
 @param [in] size ... The size of data in bytes.
 @param [in] data ... The data for hashing. If the size is zero, the data can be null.
 @param [in] seed ... A seed to get a variation of the hash function. Recommended to use the default, because this function has some known bad seeds.
 */
tshash_u64 tshash64(size_t size, const void* const data, tshash_u64 seed);

/**
 @brief Calculate the hash of data.
 @param [in] size ... The size of data in bytes.
 @param [in] data ... The data for hashing. If the size is zero, the data can be null.
 @param [in] seed ... A seed to get a variation of the hash function. Recommended to use the default, because this function has some known bad seeds.
 */
tshash_u32 tshash32(size_t size, const void* const data, tshash_u64 seed);
TSHASH_EXTERN_C_END
#endif //INC_TSHASH_H_

