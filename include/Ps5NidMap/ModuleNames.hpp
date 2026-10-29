#ifndef PS5_NID_MAP_MODULE_NAMES_HPP
#define PS5_NID_MAP_MODULE_NAMES_HPP

#include <string>
#include <string_view>

namespace Ps5NidMap {

const std::string& GetModuleName(std::string_view prxFileName);

}

#endif
