#include <Ps5NidMap/ExportLibraries.hpp>
#include <Ps5NidMap/Data/Modules.hpp>
#include <stdexcept>

namespace Ps5NidMap {

const std::vector<std::string>& GetExportLibraries(std::string_view moduleName, std::string_view nid) {
    if (moduleName.empty() || nid.empty()) throw std::invalid_argument("Empty module name or NID");
    const auto& module = *Detail::GetMappings().ExportLibraries.At(moduleName).Module;
    return module.GetLibraries(module.Nids.At(nid).Index);
}

}
