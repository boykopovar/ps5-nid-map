#ifndef PS5_NID_MAP_DATA_MAPPING_MACROS_HPP
#define PS5_NID_MAP_DATA_MAPPING_MACROS_HPP

#include <Ps5NidMap/Data/ModuleSource.hpp>

#define PS5_NID_MAP_FILES(...) ::Ps5NidMap::Detail::FileSource{std::to_array<std::string_view>({__VA_ARGS__})},
#define PS5_NID_MAP_LIBRARY(name, ...) ::Ps5NidMap::Detail::LibrarySource{name, std::to_array<::Ps5NidMap::Detail::CompiledNid>(__VA_ARGS__)},

#endif
