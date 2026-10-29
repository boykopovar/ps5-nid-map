#ifndef PS5_NID_MAP_DATA_MAPPING_MACROS_HPP
#define PS5_NID_MAP_DATA_MAPPING_MACROS_HPP

#include <initializer_list>
#include <string_view>

#define PS5_NID_MAP_FILES(...) \
    do { \
        static constexpr std::initializer_list<std::string_view> files = {__VA_ARGS__}; \
        moduleBuilder.AddFiles(files); \
    } while (false);

#define PS5_NID_MAP_LIBRARY(name, ...) \
    do { \
        static constexpr std::initializer_list<std::string_view> nids = __VA_ARGS__; \
        moduleBuilder.AddLibrary(name, nids); \
    } while (false);

#endif
