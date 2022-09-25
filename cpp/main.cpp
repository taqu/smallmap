#include "smallmap.h"
#include "tshash.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static uint64_t pcg32_state = 0x853C49E6748FEA9BULL;

static uint32_t pcg32_rotr32(uint32_t x, uint32_t r)
{
    return (x >> r) | (x << ((~r + 1) & 31U));
}

uint32_t pcg32_rand()
{
    uint64_t x = pcg32_state;
    uint32_t count = (uint32_t)(x >> 59);
    pcg32_state = x * 0x5851F42D4C957F2DULL + 0xDA3E39CB94B95BDBULL;
    x ^= x >> 18;
    return pcg32_rotr32((uint32_t)(x >> 27), count);
}

void pcg32_srand(uint64_t seed)
{
    do {
        pcg32_state = 0xDA3E39CB94B95BDBULL + seed;
    } while(0 == pcg32_state);
    pcg32_rand();
}

/**
 * @brief return [0, s)
 */
uint32_t pcg32_range_ropen(uint32_t s)
{
    uint32_t x = pcg32_rand();
    uint64_t m = (uint64_t)x * (uint64_t)s;
    uint32_t l = (uint32_t)m;
    if(l < s) {
        uint32_t t = (-s) % s;
        while(l < t) {
            x = pcg32_rand();
            m = (uint64_t)x * (uint64_t)s;
            l = (uint32_t)m;
        }
    }
    return m >> 32;
}

void shuffle(uint32_t size, char** items0, uint32_t* items1)
{
    for(uint32_t i = size; 0 != i; --i) {
        uint32_t offset = pcg32_range_ropen(i);
        assert(offset<size);
        char* tmp0 = items0[offset];
        items0[offset] = items0[i-1];
        items0[i-1] = tmp0;

        uint32_t tmp1 = items1[offset];
        items1[offset] = items1[i-1];
        items1[i-1] = tmp1;
    }
}

static bool key_constructor(smallmap* map, void* dst_key, const void* src_key)
{
    const char* src = (const char*)src_key;
    size_t len = strlen(src);
    char* dst = (char*)sm_allocate(map, (len + 1) * sizeof(char));
    if(NULL == dst) {
        return false;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
    *((char**)dst_key) = dst;
    return true;
}

static void key_move(smallmap* map, void* dst_key, const void* src_key)
{
    (void)map;
    const char** dst = (const char**)dst_key;
    const char** src = (const char**)src_key;
    *dst = *src;
    *src = NULL;
}

static void key_destructor(smallmap* map, void* key)
{
    char** dst = (char**)key;
    sm_deallocate(map, *dst);
    *dst = NULL;
}

static bool value_constructor(smallmap* map, void* dst_value, const void* src_value)
{
    (void)map;
    uint32_t* dst = (uint32_t*)dst_value;
    const uint32_t* src = (const uint32_t*)src_value;
    *dst = *src;
    return true;
}

static void value_move(smallmap* map, void* dst_value, const void* src_value)
{
    (void)map;
    int32_t* dst = (int32_t*)dst_value;
    const uint32_t* src = (const uint32_t*)src_value;
    *dst = *src;
}

static void value_destructor(smallmap* map, void* value)
{
    (void)map;
    (void)value;
}

static uint32_t hasher(const void* key)
{
    const char* str = *(const char**)(key);
    size_t len = strlen(str);
    return tshash32(len, str, TSHASH_DEFUALT_SEED);
}

static bool compare(const void* x0, const void* x1)
{
    const char* s0 = *(const char**)x0;
    const char* s1 = *(const char**)x1;
    return 0 == strcmp(s0, s1);
}

#define SAMPLE_NUM (0x1UL<<8U)

int main(void)
{
    pcg32_srand(12345);
    size_t size = sizeof(char*)*SAMPLE_NUM;
    char** keys = (char**)malloc(size);
    if(NULL == keys){
        return -1;
    }
    uint32_t* values = (uint32_t*)malloc(sizeof(uint32_t)*SAMPLE_NUM);
    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        char buffer[64] = {0};
        sprintf(buffer, "key_%010d", i);
        size_t len = strlen(buffer);
        keys[i] = (char*)malloc(len+1);
        if(NULL == keys[i]){
            return -1;
        }
        strcpy(keys[i], buffer);
        values[i] = i;
    }
    shuffle(SAMPLE_NUM, keys, values);

    smallmap* map = sm_construct(
        sizeof(char*),
        sizeof(uint32_t),
        key_constructor,
        key_move,
        key_destructor,
        value_constructor,
        value_move,
        value_destructor,
        hasher,
        compare,
        NULL, NULL);
    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        sm_add(map, keys[i], &values[i]);
    }
    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        uint32_t index = sm_find(map, keys[i]);
        assert(index != SM_INVALID);
        uint32_t value;
        bool result = sm_try_get(map, keys[i], &value);
        assert(result);
        assert(value == values[i]);
    }
    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        sm_remove(map, keys[i]);
    }
    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        uint32_t index = sm_find(map, keys[i]);
        assert(index == SM_INVALID);
    }
    sm_destruct(map);
    map = NULL;

    for(uint32_t i=0; i<SAMPLE_NUM; ++i){
        free(keys[i]);
    }
    free(values);
    free(keys);
    return 0;
}
