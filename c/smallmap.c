#include "smallmap.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

#define SM_HASH_MASK (0x7FFFFFFFUL)
#define SM_EXIST_FLAG (0x80000000UL)
#define SM_ALIGN(x) (((x) + 15UL) & ~15UL)

/**
 * @struct smallmap
 * @brief a map context
 */
struct smallmap_t
{
    uint32_t key_size_; //!< key size in bytes
    uint32_t value_size_; //!< value size in bytes
    uint64_t size_; //!< number of items
    uint64_t capacity_; //!< maximum number of items
    uint64_t mask_; //!< mask for using instead of division
    uint64_t resize_threshold_; //!< threshold for expanding the buffer
    uint32_t* hashes_; //!< hash values
    uint8_t* keys_; //!< buffer for keys
    uint8_t* values_; //!< buffer for values
    
    bool (*key_constructor_)(struct smallmap_t*, void*, const void*);
    void (*key_move_)(struct smallmap_t*, void*, const void*);
    void (*key_destructor_)(struct smallmap_t*, void*);

    bool (*value_constructor_)(struct smallmap_t*, void*, const void*);
    void (*value_move_)(struct smallmap_t*, void*, const void*);
    void (*value_destructor_)(struct smallmap_t*, void*);

    uint32_t (*hasher_)(const void*);
    bool (*compare_)(const void*, const void*);

    void* (*allocate_)(size_t);
    void (*deallocate_)(void*);
};

/**
 * @brief find an item by the calculated hash
 */
static uint32_t sm_find_(const smallmap* map, uint32_t hash, const void* key)
{
    uint32_t start = hash & map->mask_;
    hash |= SM_EXIST_FLAG;
    uint32_t pos = start;
    do {
        if(map->hashes_[pos] == hash && map->compare_(&map->keys_[pos * map->key_size_], &key)) {
            return pos;
        }
        pos = (pos + 1) & map->mask_;
    } while(pos != start);
    return SM_INVALID;
}

/**
 * @brief add an item by the calculated hash
 */
static bool sm_add_item(smallmap* map, uint32_t hash, const uint8_t* src_key, const uint8_t* src_value)
{
    uint32_t start = hash & map->mask_;
    uint32_t pos = start;
    do {
        if(SM_EXIST_FLAG != (SM_EXIST_FLAG & map->hashes_[pos])) {
            map->hashes_[pos] = hash | SM_EXIST_FLAG;
            uint8_t* key = &map->keys_[pos * map->key_size_];
            uint8_t* value = &map->values_[pos * map->value_size_];
            if(!map->key_constructor_(map, key, src_key)) {
                return false;
            }
            if(!map->value_constructor_(map, value, src_value)) {
                return false;
            }
            return true;
        }
        pos = (pos + 1) & map->mask_;
    } while(pos != start);
    return false;
}

/**
 * @brief move value of an item
 */
static void sm_move_item(smallmap* map, uint32_t hash, const uint8_t* src_key, const uint8_t* src_value)
{
    uint32_t start = hash & map->mask_;
    uint32_t pos = start;
    do {
        if(SM_EXIST_FLAG != (SM_EXIST_FLAG & map->hashes_[pos])) {
            map->hashes_[pos] = hash | SM_EXIST_FLAG;
            uint8_t* key = &map->keys_[pos * map->key_size_];
            uint8_t* value = &map->values_[pos * map->value_size_];
            map->key_move_(map, key, src_key);
            map->value_move_(map, value, src_value);
            return;
        }
        pos = (pos + 1) & map->mask_;
    } while(pos != start);
}

/**
 * @brief expand the capacity of a map
 */
static bool sm_expand(smallmap* map)
{
    uint64_t next_capacity = (map->capacity_ <= 0) ? 16 : map->capacity_ << 1;
    if(SM_INVALID <= next_capacity) {
        return false;
    }
    size_t hash_size = SM_ALIGN(next_capacity * sizeof(uint32_t));
    size_t key_size = SM_ALIGN(next_capacity * map->key_size_);
    size_t value_size = SM_ALIGN(next_capacity * map->value_size_);
    size_t total_size = hash_size + key_size + value_size;
    uint8_t* buffer = (uint8_t*)map->allocate_(total_size);
    if(NULL == buffer) {
        return false;
    }
    memset(buffer, 0, total_size);

    uint32_t* prev_hashes = map->hashes_;
    uint8_t* prev_keys = map->keys_;
    uint8_t* prev_values = map->values_;
    uint64_t prev_capacity = map->capacity_;

    map->capacity_ = next_capacity;
    map->mask_ = next_capacity - 1;
    map->resize_threshold_ = (uint64_t)(next_capacity * 0.7f);
    map->hashes_ = (uint32_t*)buffer;
    map->keys_ = buffer + hash_size;
    map->values_ = buffer + hash_size + key_size;

    for(uint32_t i = 0; i < prev_capacity; ++i) {
        if(SM_EXIST_FLAG != (prev_hashes[i] & SM_EXIST_FLAG)) {
            continue;
        }
        uint32_t hash = prev_hashes[i] & SM_HASH_MASK;
        uint8_t* key = &prev_keys[i * map->key_size_];
        uint8_t* value = &prev_values[i * map->value_size_];
        sm_move_item(map, hash, key, value);
        map->key_destructor_(map, key);
        map->value_destructor_(map, value);
    }
    map->deallocate_(prev_hashes);
    return true;
}

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
    void* (*allocate)(size_t),
    void (*deallocate)(void*))
{
    assert(NULL != key_constructor);
    assert(NULL != key_move);
    assert(NULL != key_destructor);
    assert(NULL != value_constructor);
    assert(NULL != value_move);
    assert(NULL != value_destructor);
    assert(NULL != hasher);
    assert(NULL != compare);

    if(NULL == allocate) {
        allocate = malloc;
    }
    if(NULL == deallocate) {
        deallocate = free;
    }
    smallmap* map = (smallmap*)allocate(sizeof(smallmap));
    if(NULL == map) {
        return NULL;
    }
    memset(map, 0, sizeof(smallmap));
    map->key_size_ = key_size;
    map->value_size_ = value_size;
    map->key_constructor_ = key_constructor;
    map->key_move_ = key_move;
    map->key_destructor_ = key_destructor;
    map->value_constructor_ = value_constructor;
    map->value_move_ = value_move;
    map->value_destructor_ = value_destructor;
    map->hasher_ = hasher;
    map->compare_ = compare;
    map->allocate_ = allocate;
    map->deallocate_ = deallocate;
    if(!sm_expand(map)) {
        sm_destruct(map);
        return NULL;
    }
    return map;
}

void sm_destruct(smallmap* map)
{
    if(NULL == map) {
        return;
    }
    for(uint32_t i = 0; i < map->capacity_; ++i) {
        if(SM_EXIST_FLAG != (SM_EXIST_FLAG & map->hashes_[i])) {
            continue;
        }
        map->key_destructor_(map, &map->keys_[i * map->key_size_]);
        map->value_destructor_(map, &map->values_[i * map->value_size_]);
    }
    map->deallocate_(map->hashes_);
    void (*deallocate)(void*) = map->deallocate_;
    memset(map, 0, sizeof(smallmap));
    deallocate(map);
}

void* sm_allocate(smallmap* map, size_t size)
{
    assert(NULL != map);
    return map->allocate_(size);
}

void sm_deallocate(smallmap* map, void* ptr)
{
    assert(NULL != map);
    map->deallocate_(ptr);
}

uint32_t sm_find(const smallmap* map, const void* key)
{
    assert(NULL != map);
    assert(NULL != key);
    uint32_t hash = map->hasher_(&key) & SM_HASH_MASK;
    return sm_find_(map, hash, key);
}

bool sm_try_get(const smallmap* map, const void* key, void* value)
{
    assert(NULL != map);
    assert(NULL != key);
    assert(NULL != value);
    uint32_t pos = sm_find(map, key);
    if(SM_INVALID == pos) {
        return false;
    }
    memcpy(value, &map->values_[pos * map->value_size_], map->value_size_);
    return true;
}

bool sm_add(smallmap* map, const void* key, const void* value)
{
    assert(NULL != map);
    assert(NULL != key);
    assert(NULL != value);
    uint32_t hash = map->hasher_(&key) & SM_HASH_MASK;
    if(SM_INVALID != sm_find_(map, hash, key)) {
        return false;
    }
    if(map->resize_threshold_ <= map->size_) {
        sm_expand(map);
    }
    if(!sm_add_item(map, hash, key, value)) {
        return false;
    }
    ++map->size_;
    return true;
}

void sm_remove_at(smallmap* map, uint32_t pos)
{
    assert(NULL != map);
    assert(SM_INVALID != pos);
    map->hashes_[pos] = 0;
    uint8_t* key = &map->keys_[pos * map->key_size_];
    uint8_t* value = &map->values_[pos * map->value_size_];
    map->key_destructor_(map, key);
    map->value_destructor_(map, value);
    --map->size_;
}

void sm_remove(smallmap* map, const void* key)
{
    assert(NULL != map);
    assert(NULL != key);
    uint32_t pos = sm_find(map, key);
    if(SM_INVALID == pos) {
        return;
    }
    sm_remove_at(map, pos);
}
