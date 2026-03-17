#pragma once

// std
#include <functional>



namespace KTN
{
    inline uint64_t HashString(std::string_view p_Str)
    {
        uint64_t hash = 14695981039346656037ull;

        for (char c : p_Str)
        {
            hash ^= (uint64_t)c;
            hash *= 1099511628211ull;
        }

        return hash;
    }

    template <typename T>
    inline void HashCombine(std::size_t& p_Seed, const T& p_Value)
    {
        std::hash<T> hasher;
        p_Seed ^= hasher(p_Value) + 0x9e3779b9 + (p_Seed << 6) + (p_Seed >> 2);
    }

    template <typename T, typename... Rest>
    inline void HashCombine(std::size_t& p_Seed, const T& p_Value, const Rest&... p_Rest)
    {
        HashCombine(p_Seed, p_Value);
        (HashCombine(p_Seed, p_Rest), ...);
    }

} // namespace KTN
