#include <Ps5NidMap/Data/Modules.hpp>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

template<class TMap>
struct Query {
    const TMap* Map;
    std::string_view Key;
};

template<class TMap>
const typename TMap::mapped_type& Lookup(const Query<TMap>& query) {
    const auto found = query.Map->find(query.Key);
    if (found == query.Map->end()) throw std::out_of_range("Benchmark key not found");
    return found->second;
}

template<class TMap>
void Measure(std::string_view name, std::vector<Query<TMap>>& queries) {
    if (queries.empty()) throw std::runtime_error("Cannot measure an empty dictionary");
    std::mt19937 generator(0);
    std::shuffle(queries.begin(), queries.end(), generator);
    volatile std::size_t observedSize = 0;
    for (const auto& query : queries) observedSize = Lookup(query).size();
    constexpr std::size_t MinimumLookups = 2000000;
    const auto repetitions = (MinimumLookups + queries.size() - 1) / queries.size();
    const auto lookups = repetitions * queries.size();
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t repetition = 0; repetition < repetitions; ++repetition) {
        for (const auto& query : queries) observedSize = Lookup(query).size();
    }
    const auto elapsed = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
    static_cast<void>(observedSize);
    std::cout << name << ": " << elapsed / static_cast<double>(lookups) << " ns/lookup; keys=" << queries.size() << "; lookups=" << lookups << '\n';
}

}

int main() {
    const auto& mappings = Ps5NidMap::Detail::GetMappings();
    using ModuleMap = std::remove_cvref_t<decltype(mappings.ModuleNames)>;
    using ExportMap = std::remove_cvref_t<decltype(mappings.ExportLibraries)>;
    using NidMap = ExportMap::mapped_type;
    std::vector<Query<ModuleMap>> moduleQueries;
    std::vector<Query<ExportMap>> exportQueries;
    std::vector<Query<NidMap>> nidQueries;
    moduleQueries.reserve(mappings.ModuleNames.size());
    exportQueries.reserve(mappings.ExportLibraries.size());
    for (const auto& entry : mappings.ModuleNames) moduleQueries.push_back({&mappings.ModuleNames, entry.first});
    for (const auto& module : mappings.ExportLibraries) {
        exportQueries.push_back({&mappings.ExportLibraries, module.first});
        for (const auto& entry : module.second) nidQueries.push_back({&module.second, entry.first});
    }
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean successful lookup time; shuffled keys, after warmup, loop and result-size read included\n";
    Measure("PRX filename -> module", moduleQueries);
    Measure("Module -> NID dictionary", exportQueries);
    Measure("NID -> libraries", nidQueries);
}
