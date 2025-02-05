#include "ktnpch.h"
#include "Utils.h"


namespace KTN::Utils
{
	KTN_API uint32_t DataTypeSize(DataType p_Type)
	{
		switch (p_Type)
		{
			case DataType::Float:		return 4;
			case DataType::Float2:		return 4 * 2;
			case DataType::Float3:		return 4 * 3;
			case DataType::Float4:		return 4 * 4;
			case DataType::Float3x3:	return 4 * 3 * 3;
			case DataType::Float4x4:	return 4 * 4 * 4;
			case DataType::Int:			return 4;
			case DataType::Int2:		return 4 * 2;
			case DataType::Int3:		return 4 * 3;
			case DataType::Int4:		return 4 * 4;
			case DataType::Int3x3:		return 4 * 3 * 3;
			case DataType::Int4x4:		return 4 * 4 * 4;
			case DataType::Bool:		return 1;
		}

		KTN_CORE_ERROR("Unknown ShaderDataType!");
		return 0;
	}

} // namespace KTN::Utils
