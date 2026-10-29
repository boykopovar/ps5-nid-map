#include <Ps5NidMap/Data/Modules.hpp>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <string>
#include <type_traits>
#include <vector>

namespace {

std::string EncodeNid(std::uint64_t number) {
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
    std::string result(11, 'A');
    result[10] = alphabet[(number & 15) << 2];
    number >>= 4;
    for (std::size_t index = 10; index > 0; --index) {
        result[index - 1] = alphabet[number & 63];
        number >>= 6;
    }
    return result;
}

template<class TMap>
struct Query {
    const TMap* Map;
    std::string_view Key;
};

template<class TMap>
void Measure(std::string_view name, std::vector<Query<TMap>>& queries) {
    if (queries.empty()) throw std::runtime_error("Cannot measure an empty dictionary");
    std::mt19937 generator(0);
    std::shuffle(queries.begin(), queries.end(), generator);
    volatile std::size_t observedSize = 0;
    for (const auto& query : queries) observedSize = query.Map->At(query.Key).Size();
    constexpr std::size_t MinimumLookups = 2000000;
    const auto repetitions = (MinimumLookups + queries.size() - 1) / queries.size();
    const auto lookups = repetitions * queries.size();
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t repetition = 0; repetition < repetitions; ++repetition) {
        for (const auto& query : queries) observedSize = query.Map->At(query.Key).Size();
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
    using NidMap = Ps5NidMap::Detail::Dictionary<Ps5NidMap::Detail::LibraryGroup, true>;
    std::vector<Query<ModuleMap>> moduleQueries;
    std::vector<Query<ExportMap>> exportQueries;
    std::vector<Query<NidMap>> nidQueries;
    std::size_t nidCount = 0;
    for (const auto& module : mappings.ExportLibraries.Entries()) nidCount += module.Value.Module->Nids.Size();
    std::vector<std::string> nidKeys;
    nidKeys.reserve(nidCount);
    nidQueries.reserve(nidCount);
    moduleQueries.reserve(mappings.ModuleNames.Size());
    exportQueries.reserve(mappings.ExportLibraries.Size());
    for (const auto& entry : mappings.ModuleNames.Entries()) moduleQueries.push_back({&mappings.ModuleNames, entry.Key});
    for (const auto& module : mappings.ExportLibraries.Entries()) {
        exportQueries.push_back({&mappings.ExportLibraries, module.Key});
        for (const auto& entry : module.Value.Module->Nids.Entries()) {
            nidKeys.push_back(EncodeNid(entry.Key));
            nidQueries.push_back({&module.Value.Module->Nids, nidKeys.back()});
        }
    }
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean successful lookup time; shuffled keys, after warmup, loop and result-size read included\n";
    Measure("PRX filename -> module", moduleQueries);
    Measure("Module -> NID dictionary", exportQueries);
    Measure("NID -> libraries", nidQueries);
}
