#pragma once
#include "Koten/Core/Base.h"


extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace KTN
{
	class KTN_API ScriptEngine
	{
		public:
			static void Init();
			static void Shutdown();

			static bool LoadAssembly(const std::string& p_Path);
	};

	class KTN_API ScriptClass
	{
		public:
			ScriptClass() = default;
			ScriptClass(const std::string& p_Namespace, const std::string& p_Name);

			MonoObject* Instantiate();
			MonoMethod* GetMethod(const std::string& p_Name, int p_ParameterCount);
			MonoObject* InvokeMethod(MonoObject* p_Instance, MonoMethod* p_Method, void** p_Params = nullptr);

		private:
			std::string m_Namespace = "";
			std::string m_Name = "";

			MonoClass* m_Class = nullptr;
	};

} // namespace KTN