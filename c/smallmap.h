#ifndef INC_SMALLMAP_H_
#define INC_SMALLMAP_H_
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct smallmap_t;
typedef struct smallmap_t smallmap;
#define SM_INVALID (0xFFFFFFFFUL) //!< Invalid ID

/**
 * @brief construct a map context
 * @param [in] key_size ... size of key in bytes
 * @param [in] value_size ... size of value in bytes
 * @param [in] key_constructor ...
 * @param [in] key_move ...
 * @param [in] key_destructor ...
 * @param [in] value_constructor ...
 * @param [in] value_move ...
 * @param [in] value_destructor ...
 * @param [in] hasher ...
 * @param [in] compare ...
 * @param [in] allocate ...
 * @param [in] deallocate ...
 */
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

/**
 * @brief destruct a map context
 */
void sm_destruct(smallmap* map);

/**
 * @brief allocate memory with the map's allocator
 * @param [in] map ... the owner of allocator
 * @param [in] size ... size of allocation
 */
void* sm_allocate(smallmap* map, size_t size);

/**
 * @brief deallocate memory which allocated by the map's allocator
 * @param [in] map ... the owner of allocator
 * @param [in] ptr ... the pointer to the allocated memory
 */
void sm_deallocate(smallmap* map, void* ptr);

/**
 * @brief find an item
 * @return position of the found item, SM_INVALID if cannot find
 * @param [in] map ... a map context
 * @param [in] key ... a target key
 */
uint32_t sm_find(const smallmap* map, const void* key);

/**
 * @brief find an item
 * @return true if can find
 * @param [in] map ... a map context
 * @param [in] key ... a target key
 * @param [out] value ... copy the value, if found
 */
bool sm_try_get(const smallmap* map, const void* key, void* value);

/**
 * @brief add an item to a map
 * @return result of adding
 * @param [in] map ... a map context
 * @param [in] key ... a target key
 * @param [out] value ... copy the value, if found
 */
bool sm_add(smallmap* map, const void* key, const void* value);

/**
 * @brief remove an item from a map
 * @param [in] map ... a map context
 * @param [in] pos ... the target item's position which can be found by sm_find
 */
void sm_remove_at(smallmap* map, uint32_t pos);

/**
 * @brief remove an item from a map
 * @param [in] map ... a map context
 * @param [in] key ... a target key
 */
void sm_remove(smallmap* map, const void* key);
#endif //INC_SMALLMAP_H_
