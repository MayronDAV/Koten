#pragma once
#include "Base.h"


namespace KTN
{
	// Non-owning raw buffer class
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;

		Buffer(uint64_t p_Size)
		{
			Allocate(p_Size);
		}

		Buffer(const void* p_Data, uint64_t p_Size)
			: Data((uint8_t*)p_Data), Size(p_Size)
		{
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer p_Other)
		{
			Buffer result(p_Other.Size);
			memcpy(result.Data, p_Other.Data, p_Other.Size);
			return result;
		}

		void Allocate(uint64_t p_Size)
		{
			Release();

			Data = (uint8_t*)malloc(p_Size);
			Size = p_Size;
		}

		void Release()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		operator bool() const
		{
			return (bool)Data;
		}
	};

	struct ScopedBuffer
	{
		ScopedBuffer(Buffer p_Buffer)
			: m_Buffer(p_Buffer)
		{
		}

		ScopedBuffer(uint64_t p_Size)
			: m_Buffer(p_Size)
		{
		}

		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.Data; }
		uint64_t Size() { return m_Buffer.Size; }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }
	private:
		Buffer m_Buffer;
	};
}