#include "ktnpch.h"
#include "ScriptGlue.h"

// lib
#include <mono/metadata/object.h>


namespace KTN
{
	#define KTN_ADD_INTERNAL_CALL(Name) mono_add_internal_call("KTN.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* p_String, int p_Parameter)
	{
		char* cStr = mono_string_to_utf8(p_String);
		std::string str(cStr);
		mono_free(cStr);
		KTN_TRACE("{0}, {1}", str, p_Parameter);
	}

	static void NativeLog_Vector(glm::vec3* p_Parameter, glm::vec3* p_OutResult)
	{
		auto value = *p_Parameter;
		KTN_WARN("Value: ({}, {}, {})", value.x, value.y, value.z);
		*p_OutResult = glm::normalize(*p_Parameter);
	}

	static float NativeLog_VectorDot(glm::vec3* p_Parameter)
	{
		auto value = *p_Parameter;
		KTN_WARN("Value: ({}, {}, {})", value.x, value.y, value.z);
		return glm::dot(*p_Parameter, *p_Parameter);
	}

	void ScriptGlue::RegisterFunctions()
	{
		KTN_ADD_INTERNAL_CALL(NativeLog);
		KTN_ADD_INTERNAL_CALL(NativeLog_Vector);
		KTN_ADD_INTERNAL_CALL(NativeLog_VectorDot);
	}

} // namespace KTN