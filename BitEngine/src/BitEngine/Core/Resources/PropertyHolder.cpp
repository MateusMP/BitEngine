#include "BitEngine/Core/Resources/PropertyHolder.h"

namespace BitEngine {

template <>
void PropertyHolder::read<u8>(const char* name, u8* type)
{
    u32 x;
    this->_read(name, &x);
    *type = (u8)x;
}
}