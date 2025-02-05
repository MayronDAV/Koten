#pragma once

#include "Base.h"
#include "Definitions.h"


namespace KTN
{
	class KTN_API Engine
	{
		public:
			static RenderAPI GetAPI() { return s_API; }
			static void SetAPI(RenderAPI p_API) { s_API = p_API; }

		private:
			static RenderAPI s_API;
	};

} // namespace KTN
