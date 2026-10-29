#include <Ps5NidMap/Data/Modules.hpp>
#include <initializer_list>

namespace Ps5NidMap::Detail {

#include "ModuleRegistry.inc"

const Mappings& GetMappings() {
    static constexpr Mappings mappings{FileDictionary.View(), ModuleDictionary.View()};
    return mappings;
}

}
