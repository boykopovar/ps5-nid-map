#ifndef PS5_NID_MAP_HASH_PERFECT_HASH_HPP
#define PS5_NID_MAP_HASH_PERFECT_HASH_HPP

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace Ps5NidMap::Detail {

constexpr std::uint64_t Mix(std::uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

constexpr std::uint64_t HashName(std::string_view key) {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char character : key) hash = (hash ^ character) * 1099511628211ULL;
    return hash;
}

inline constexpr auto NidDigits = [] {
    std::array<unsigned char, 256> digits{};
    digits.fill(255);
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
    for (std::size_t index = 0; index < alphabet.size(); ++index) digits[static_cast<unsigned char>(alphabet[index])] = static_cast<unsigned char>(index);
    return digits;
}();

constexpr bool DecodeNid(std::string_view key, std::uint64_t& result) {
    if (key.size() != 11) return false;
    result = 0;
    for (std::size_t index = 0; index < 10; ++index) {
        const auto digit = NidDigits[static_cast<unsigned char>(key[index])];
        if (digit == 255) return false;
        result = (result << 6) | digit;
    }
    const auto last = NidDigits[static_cast<unsigned char>(key[10])];
    if (last == 255 || (last & 3) != 0) return false;
    result = (result << 4) | (last >> 2);
    return true;
}

struct Bucket {
    std::uint32_t Offset = 0;
    std::uint32_t Size = 0;
    std::uint64_t Seed = 0;
};

template<class TValue, bool TNid = false>
struct Entry {
    using KeyType = std::conditional_t<TNid, std::uint64_t, std::string_view>;
    KeyType Key;
    TValue Value;
};

template<class TValue, bool TNid = false>
class Dictionary {
public:
    constexpr Dictionary(std::span<const Entry<TValue, TNid>> entries, std::span<const Bucket> buckets, std::span<const std::uint32_t> slots, std::uint64_t seed) : entries(entries), buckets(buckets), slots(slots), seed(seed) {}
    constexpr std::span<const Entry<TValue, TNid>> Entries() const { return entries; }
    constexpr std::size_t Size() const { return entries.size(); }
    const TValue& At(std::string_view key) const {
        const auto found = Find(key);
        if (!found) throw std::out_of_range("Unknown dictionary key");
        return *found;
    }
    constexpr const TValue* Find(std::string_view key) const {
        if constexpr (TNid) {
            std::uint64_t number;
            if (!DecodeNid(key, number)) return nullptr;
            return findKey(number, number);
        } else return findKey(HashName(key), key);
    }
    constexpr const TValue* FindNumeric(std::uint64_t key) const requires TNid { return findKey(key, key); }

private:
    constexpr const TValue* findKey(std::uint64_t hash, typename Entry<TValue, TNid>::KeyType key) const {
        if (entries.empty()) return nullptr;
        const auto bucketIndex = (TNid ? hash : Mix(hash + seed)) & (buckets.size() - 1);
        const auto& bucket = buckets[bucketIndex];
        if (bucket.Size == 0) return nullptr;
        const auto index = bucket.Size == 1 ? bucket.Offset : slots[bucket.Offset + (Mix(hash + bucket.Seed) & (bucket.Size - 1))];
        if (index == std::numeric_limits<std::uint32_t>::max()) return nullptr;
        const auto& entry = entries[index];
        return entry.Key == key ? &entry.Value : nullptr;
    }

    std::span<const Entry<TValue, TNid>> entries;
    std::span<const Bucket> buckets;
    std::span<const std::uint32_t> slots;
    std::uint64_t seed;
};

template<class TValue, std::size_t TSize, bool TNid = false>
struct DictionaryStorage {
    std::array<Entry<TValue, TNid>, TSize> Entries;
    std::array<Bucket, std::bit_ceil(TSize)> Buckets{};
    std::array<std::uint32_t, TSize * 4> Slots{};
    std::uint64_t Seed = 0;
    constexpr Dictionary<TValue, TNid> View() const { return {Entries, Buckets, Slots, Seed}; }
};

template<bool TNid = false, class TValue, std::size_t TSize>
consteval auto BuildDictionary(std::array<Entry<TValue, TNid>, TSize> entries) {
    static_assert(TSize <= std::numeric_limits<std::uint32_t>::max() / 4);
    DictionaryStorage<TValue, TSize, TNid> result{entries};
    if constexpr (TSize != 0) {
        constexpr auto Empty = std::numeric_limits<std::uint32_t>::max();
        std::array<std::uint64_t, TSize> hashes{};
        for (std::size_t index = 0; index < TSize; ++index) {
            if constexpr (TNid) hashes[index] = entries[index].Key;
            else {
                if (entries[index].Key.empty()) throw "Empty perfect hash key";
                hashes[index] = HashName(entries[index].Key);
            }
        }
        constexpr auto BucketCount = std::bit_ceil(TSize);
        std::array<std::uint32_t, BucketCount> counts{};
        bool primaryFound = false;
        for (std::uint64_t attempt = 1; attempt <= 4096; ++attempt) {
            counts.fill(0);
            for (const auto hash : hashes) ++counts[(TNid ? hash : Mix(hash + attempt)) & (BucketCount - 1)];
            std::size_t capacity = 0;
            for (const auto count : counts) capacity += count < 2 ? 0 : std::bit_ceil(static_cast<std::size_t>(count) * count);
            if (capacity <= result.Slots.size()) {
                result.Seed = attempt;
                primaryFound = true;
                break;
            }
        }
        if (!primaryFound) throw "Perfect hash primary seed search exhausted";
        std::array<std::uint32_t, BucketCount + 1> starts{};
        for (std::size_t index = 0; index < BucketCount; ++index) starts[index + 1] = starts[index] + counts[index];
        auto cursors = starts;
        std::array<std::uint32_t, TSize> order{};
        for (std::size_t index = 0; index < TSize; ++index) order[cursors[(TNid ? hashes[index] : Mix(hashes[index] + result.Seed)) & (BucketCount - 1)]++] = static_cast<std::uint32_t>(index);
        result.Slots.fill(Empty);
        std::uint32_t offset = 0;
        for (std::size_t bucketIndex = 0; bucketIndex < BucketCount; ++bucketIndex) {
            const auto count = counts[bucketIndex];
            auto& bucket = result.Buckets[bucketIndex];
            if (count == 0) continue;
            if (count == 1) {
                bucket = {order[starts[bucketIndex]], 1, 0};
                continue;
            }
            bucket.Offset = offset;
            bucket.Size = std::bit_ceil(count * count);
            offset += bucket.Size;
            bool found = false;
            for (std::uint64_t attempt = 1; attempt <= 65536; ++attempt) {
                for (std::size_t slot = bucket.Offset; slot < offset; ++slot) result.Slots[slot] = Empty;
                bool collision = false;
                for (auto position = starts[bucketIndex]; position < starts[bucketIndex + 1]; ++position) {
                    const auto index = order[position];
                    auto& slot = result.Slots[bucket.Offset + (Mix(hashes[index] + attempt) & (bucket.Size - 1))];
                    if (slot != Empty) {
                        if (hashes[slot] == hashes[index]) throw "Duplicate key or identical key hashes";
                        collision = true;
                        break;
                    }
                    slot = index;
                }
                if (!collision) {
                    bucket.Seed = attempt;
                    found = true;
                    break;
                }
            }
            if (!found) throw "Perfect hash secondary seed search exhausted";
        }
        for (const auto& entry : entries) {
            if constexpr (TNid) {
                if (!result.View().FindNumeric(entry.Key)) throw "Perfect hash verification failed";
            } else if (!result.View().Find(entry.Key)) throw "Perfect hash verification failed";
        }
    }
    return result;
}

template<std::size_t TSize>
struct SecondaryStorage {
    static_assert(TSize > 1);
    std::array<std::uint32_t, std::bit_ceil(TSize * TSize)> Slots;
    std::uint64_t Seed;
};

template<std::size_t TSize>
consteval auto BuildSecondary(const std::array<std::uint64_t, TSize>& keys, std::uint32_t firstIndex) {
    SecondaryStorage<TSize> result{};
    constexpr auto Empty = std::numeric_limits<std::uint32_t>::max();
    for (std::uint64_t seed = 1; seed <= 65536; ++seed) {
        result.Slots.fill(Empty);
        bool collision = false;
        for (std::size_t index = 0; index < keys.size(); ++index) {
            auto& slot = result.Slots[Mix(keys[index] + seed) & (result.Slots.size() - 1)];
            if (slot != Empty) {
                if (keys[slot - firstIndex] == keys[index]) throw "Duplicate secondary key";
                collision = true;
                break;
            }
            slot = firstIndex + static_cast<std::uint32_t>(index);
        }
        if (!collision) {
            result.Seed = seed;
            return result;
        }
    }
    throw "Secondary hash seed search exhausted";
}

}

#endif
