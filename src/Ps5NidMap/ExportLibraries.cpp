#include <Ps5NidMap/ExportLibraries.hpp>
#include <Ps5NidMap/Data/Modules.hpp>
#include <stdexcept>

namespace Ps5NidMap {

const std::vector<std::string>& GetExportLibraries(std::string_view moduleName, std::string_view nid) {
    if (moduleName.empty() || nid.empty()) throw std::invalid_argument("Empty module name or NID");
    const auto& modules = Detail::GetMappings().ExportLibraries;
    const auto module = modules.find(moduleName);
    if (module == modules.end()) throw std::out_of_range("Unknown module: " + std::string(moduleName));
    const auto found = module->second.find(nid);
    if (found == module->second.end()) throw std::out_of_range("Unknown NID in module " + std::string(moduleName) + ": " + std::string(nid));
    return found->second;
}

}
