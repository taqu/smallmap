#ifndef INC_SMALLMAP_H_
#define INC_SMALLMAP_H_
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct smallmap_t;
typedef struct smallmap_t smallmap;

smallmap* sm_construct(
        uint32_t key_size,
        uint32_t value_size,
        bool (*key_constructor)(smallmap*, void*, const void*),
        void (*key_move)(smallmap*, void*, const void*),
        void (*key_destructor)(smallmap*, void*),
        bool (*value_constructor)(smallmap*, void*, const void*),
        void (*value_move)(smallmap*, void*, const void*),
        void (*value_destructor)(smallmap*, void*),
        uint32_t (*hasher)(const void*),
        bool (*compare)(const void*, const void*),
        void*(*allocate)(size_t),
        void(*deallocate)(void*));

void sm_destruct(smallmap* map);

void* sm_allocate(smallmap* map, size_t size);
void sm_deallocate(smallmap* map, void* ptr);

uint32_t sm_find(const smallmap* map, const void* key);
bool sm_try_get(const smallmap* map, const void* key, void** value);
bool sm_add(smallmap* map, const void* key, const void* value);

void sm_remove_at(smallmap* map, uint32_t pos);
void sm_remove(smallmap* map, const void* key);
#endif //INC_SMALLMAP_H_

