#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"



namespace KTN::Utils
{
	inline const char* ScriptFieldTypeToString(ScriptFieldType p_Type)
	{
		KTN_PROFILE_FUNCTION();

		#define SCRIPT_FIELD_TYPE_CASE(type) case ScriptFieldType::type: return #type

		switch (p_Type)
		{
			SCRIPT_FIELD_TYPE_CASE(None);
			SCRIPT_FIELD_TYPE_CASE(Float);
			SCRIPT_FIELD_TYPE_CASE(Double);
			SCRIPT_FIELD_TYPE_CASE(Bool);
			SCRIPT_FIELD_TYPE_CASE(Char);
			SCRIPT_FIELD_TYPE_CASE(Short);
			SCRIPT_FIELD_TYPE_CASE(Int);
			SCRIPT_FIELD_TYPE_CASE(Long);
			SCRIPT_FIELD_TYPE_CASE(Byte);
			SCRIPT_FIELD_TYPE_CASE(UShort);
			SCRIPT_FIELD_TYPE_CASE(UInt);
			SCRIPT_FIELD_TYPE_CASE(ULong);
			SCRIPT_FIELD_TYPE_CASE(String);
			SCRIPT_FIELD_TYPE_CASE(Vector2);
			SCRIPT_FIELD_TYPE_CASE(Vector3);
			SCRIPT_FIELD_TYPE_CASE(Vector4);
			SCRIPT_FIELD_TYPE_CASE(Entity);
			default: return "<Invalid>";
		}

		#undef SCRIPT_FIELD_TYPE_CASE
	}

	inline ScriptFieldType ScriptFieldTypeFromString(std::string_view p_Type)
	{
		#define SCRIPT_FIELD_TYPE(type) if (p_Type == #type) return ScriptFieldType::type;

		SCRIPT_FIELD_TYPE(None);
		SCRIPT_FIELD_TYPE(Float);
		SCRIPT_FIELD_TYPE(Double);
		SCRIPT_FIELD_TYPE(Bool);
		SCRIPT_FIELD_TYPE(Char);
		SCRIPT_FIELD_TYPE(Short);
		SCRIPT_FIELD_TYPE(Int);
		SCRIPT_FIELD_TYPE(Long);
		SCRIPT_FIELD_TYPE(Byte);
		SCRIPT_FIELD_TYPE(UShort);
		SCRIPT_FIELD_TYPE(UInt);
		SCRIPT_FIELD_TYPE(ULong);
		SCRIPT_FIELD_TYPE(String);
		SCRIPT_FIELD_TYPE(Vector2);
		SCRIPT_FIELD_TYPE(Vector3);
		SCRIPT_FIELD_TYPE(Vector4);
		SCRIPT_FIELD_TYPE(Entity);


		#undef SCRIPT_FIELD_TYPE
		KTN_CORE_ERROR("Unknown ScriptFieldType: {}", p_Type);
		return ScriptFieldType::None;
	}

} // namespace KTN::Utils