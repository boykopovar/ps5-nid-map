#ifndef PS5_NID_MAP_DATA_MAPPING_MACROS_HPP
#define PS5_NID_MAP_DATA_MAPPING_MACROS_HPP

#define PS5_NID_MAP_FILES(...) moduleBuilder.AddFiles({__VA_ARGS__});
#define PS5_NID_MAP_LIBRARY(name, ...) moduleBuilder.AddLibrary(name, __VA_ARGS__);

#endif
