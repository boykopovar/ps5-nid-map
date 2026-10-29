#include <Ps5NidMap/ModuleNames.hpp>
#include <Ps5NidMap/Data/Modules.hpp>
#include <stdexcept>

namespace Ps5NidMap {

const std::string& GetModuleName(std::string_view prxFileName) {
    if (prxFileName.empty()) throw std::invalid_argument("Empty PRX filename");
    return Detail::GetMappings().ModuleNames.At(prxFileName).Module->GetName();
}

}
