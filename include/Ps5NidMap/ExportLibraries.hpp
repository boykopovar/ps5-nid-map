#ifndef PS5_NID_MAP_EXPORT_LIBRARIES_HPP
#define PS5_NID_MAP_EXPORT_LIBRARIES_HPP

#include <string>
#include <string_view>
#include <vector>

namespace Ps5NidMap {

const std::vector<std::string>& GetExportLibraries(std::string_view moduleName, std::string_view nid);

}

#endif
