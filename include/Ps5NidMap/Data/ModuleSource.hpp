#ifndef PS5_NID_MAP_DATA_MODULE_SOURCE_HPP
#define PS5_NID_MAP_DATA_MODULE_SOURCE_HPP

#include <Ps5NidMap/Data/Modules.hpp>
#include <tuple>

namespace Ps5NidMap::Detail {

struct CompiledNid {
    std::uint64_t Value;
    consteval CompiledNid(const char* nid) : Value(0) {
        if (!DecodeNid(nid, Value)) throw "Invalid NID";
    }
};

template<std::size_t TSize>
struct FileSource {
    std::array<std::string_view, TSize> Files;
};

template<std::size_t TSize>
struct LibrarySource {
    std::string_view Name;
    std::array<CompiledNid, TSize> Nids;
};

}

#endif
