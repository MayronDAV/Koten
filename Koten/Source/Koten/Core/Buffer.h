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

		void Write(const void* p_Data, uint64_t p_Size)
		{
			uint8_t* newData = (uint8_t*)malloc(Size + p_Size);

			if (Data)
			{
				memcpy(newData, Data, Size);
				free(Data);
			}

			memcpy(newData + Size, p_Data, p_Size);

			Data = newData;
			Size = Size + p_Size;
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
		uint64_t Size() const { return m_Buffer.Size; }
		const Buffer& GetBuffer() { return m_Buffer; }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }
	private:
		Buffer m_Buffer;
	};

	struct BufferReader
	{
		BufferReader(const Buffer& p_Buffer)
			: m_Buffer(p_Buffer) {}

		template<typename T>
		T Read()
		{
			if (!m_Buffer)
			{
				KTN_CORE_ERROR("Buffer is null!");
				return T();
			}

			if (End())
			{
				KTN_CORE_ERROR("End of buffer reached!");
				return T();
			}

			T value;
			ReadBytes(&value, sizeof(T));
			return value;
		}

		void ReadBytes(void* p_Dest, uint64_t p_Size)
		{
			if (!m_Buffer)
			{
				KTN_CORE_ERROR("Buffer is null!");
			}

			if (End())
			{
				KTN_CORE_ERROR("End of buffer reached!");
			}

			memcpy(p_Dest, m_Buffer.Data + m_Offset, p_Size);
			m_Offset += p_Size;
		}

		bool End() const { return m_Offset >= m_Buffer.Size; }

	private:
		uint64_t m_Offset = 0;
		const Buffer& m_Buffer;
	};

}