#ifndef INC_SMALLMAP_H_
#define INC_SMALLMAP_H_
#include <cstdint>

namespace sm
{
template<class Key, class Value, class MemoryAllocator>
class smallmap
{
public:
    using this_type = smallmap<Key, Value, MemoryAllocator>;
};
}
#endif //INC_SMALLMAP_H_
