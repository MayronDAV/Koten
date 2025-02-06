#pragma once

// std
#include <functional>



namespace KTN
{
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
