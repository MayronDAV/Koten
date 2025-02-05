#pragma once
#include "Koten/Graphics/GraphicsContext.h"


namespace KTN
{
	class GLContext : public GraphicsContext
	{
		public:
			GLContext();
			~GLContext() override = default;

			void Init(void* p_Window, const char* p_Name) override;

			void SwapBuffer() override;

			void SetVsync(bool p_Value) override;

		private:
			void* m_Window = nullptr;
	};


} // namespace KTN