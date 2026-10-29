#ifndef PS5_NID_MAP_DATA_MODULES_HPP
#define PS5_NID_MAP_DATA_MODULES_HPP

#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Ps5NidMap::Detail {

struct Mappings {
    std::map<std::string, std::string, std::less<>> ModuleNames;
    std::map<std::string, std::map<std::string, std::vector<std::string>, std::less<>>, std::less<>> ExportLibraries;
};

class ModuleBuilder {
public:
    ModuleBuilder(Mappings& mappings, std::string_view moduleName);
    void AddFiles(std::initializer_list<std::string_view> files);
    void AddLibrary(std::string_view libraryName, std::initializer_list<std::string_view> nids);

private:
    Mappings& mappings;
    std::string moduleName;
    std::vector<std::string> libraryNames;
};

const Mappings& GetMappings();

}

#endif
