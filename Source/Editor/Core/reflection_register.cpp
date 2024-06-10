#include <cassert>

#include "Reflection/json.h"
#include "Reflection/reflection/reflection.h"
#include "reflection_register.h"
#include "Reflection/serializer/serializer.h"
#include "../_generated/reflection/all_reflection.h"
#include "../_generated/serializer/all_serializer.ipp"

namespace Reflection
{
    void TypeMetaRegister::metaUnregister() { TypeMetaRegisterinterface::unregisterAll(); }
} // namespace Reflection