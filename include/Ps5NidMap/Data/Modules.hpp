#ifndef PS5_NID_MAP_DATA_MODULES_HPP
#define PS5_NID_MAP_DATA_MODULES_HPP

#include <Ps5NidMap/Hash/PerfectHash.hpp>
#include <string>
#include <vector>

namespace Ps5NidMap::Detail {

struct LibraryGroup {
    std::uint32_t Index;
    std::uint32_t Count;
    constexpr std::size_t Size() const { return Count; }
};

struct ModuleData {
    std::string_view Name;
    Dictionary<LibraryGroup, true> Nids;
    const std::string& (*GetName)();
    const std::vector<std::string>& (*GetLibraries)(std::size_t index);
};

struct ModuleReference {
    const ModuleData* Module;
    constexpr std::size_t Size() const { return Module->Nids.Size(); }
};

struct Mappings {
    Dictionary<ModuleReference> ModuleNames;
    Dictionary<ModuleReference> ExportLibraries;
};

const Mappings& GetMappings();

}

#endif
