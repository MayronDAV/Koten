#pragma once

#include "Base.h"

namespace KTN
{
	class KTN_API UUID
	{
		public:
			UUID();
			UUID(uint64_t p_UUID);
			UUID(const UUID&) = default;

			operator uint64_t() const { return m_UUID; }

		private:
			uint64_t m_UUID;
	};

} // namespace KTN

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<KTN::UUID>
	{
		std::size_t operator()(const KTN::UUID& p_UUID) const
		{
			return (uint64_t)p_UUID;
		}
	};

} // namespace std
