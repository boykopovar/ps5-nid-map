#include <Ps5NidMap/Data/Modules.hpp>
#include <Ps5NidMap/Data/MappingMacros.hpp>
#include <algorithm>
#include <stdexcept>

namespace Ps5NidMap::Detail {

ModuleBuilder::ModuleBuilder(Mappings& mappings, std::string_view moduleName) : mappings(mappings), moduleName(moduleName) {
    if (moduleName.empty()) throw std::invalid_argument("Empty module name");
    if (!mappings.ExportLibraries.try_emplace(this->moduleName).second) throw std::invalid_argument("Duplicate module: " + this->moduleName);
}

void ModuleBuilder::AddFiles(std::initializer_list<std::string_view> files) {
    if (files.size() == 0) throw std::invalid_argument("Empty PRX filename list: " + moduleName);
    for (const auto file : files) {
        if ((!file.ends_with(".prx") && !file.ends_with(".sprx")) || file.find_first_of("/\\") != std::string_view::npos || file.starts_with('.')) throw std::invalid_argument("Invalid PRX filename: " + std::string(file));
        if (!mappings.ModuleNames.emplace(file, moduleName).second) throw std::invalid_argument("Duplicate PRX filename: " + std::string(file));
    }
}

void ModuleBuilder::AddLibrary(std::string_view libraryName, std::initializer_list<std::string_view> nids) {
    if (libraryName.empty() || nids.size() == 0) throw std::invalid_argument("Empty library name or NID list: " + moduleName);
    if (std::find(libraryNames.begin(), libraryNames.end(), libraryName) != libraryNames.end()) throw std::invalid_argument("Duplicate library: " + std::string(libraryName));
    libraryNames.emplace_back(libraryName);
    auto& exports = mappings.ExportLibraries.at(moduleName);
    for (const auto nid : nids) {
        if (nid.size() != 11 || nid.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-") != std::string_view::npos) throw std::invalid_argument("Invalid NID: " + std::string(nid));
        auto& libraries = exports[std::string(nid)];
        if (std::find(libraries.begin(), libraries.end(), libraryName) != libraries.end()) throw std::invalid_argument("Duplicate NID in library " + std::string(libraryName) + ": " + std::string(nid));
        libraries.emplace_back(libraryName);
    }
}

const Mappings& GetMappings() {
    static const Mappings mappings = [] {
        Mappings mappings;
#include "ModuleIncludes.inc"
        return mappings;
    }();
    return mappings;
}

}

#undef PS5_NID_MAP_FILES
#undef PS5_NID_MAP_LIBRARY
