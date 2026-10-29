#include <Ps5NidMap/ModuleNames.hpp>
#include <Ps5NidMap/Data/Modules.hpp>
#include <stdexcept>

namespace Ps5NidMap {

const std::string& GetModuleName(std::string_view prxFileName) {
    if (prxFileName.empty()) throw std::invalid_argument("Empty PRX filename");
    const auto& names = Detail::GetMappings().ModuleNames;
    const auto found = names.find(prxFileName);
    if (found == names.end()) throw std::out_of_range("Unknown PRX filename: " + std::string(prxFileName));
    return found->second;
}

}
