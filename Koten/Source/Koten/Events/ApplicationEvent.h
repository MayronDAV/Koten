#pragma once

#include "Event.h"


namespace KTN
{
	class KTN_API AppTickEvent : public Event
	{
		public:
			AppTickEvent() = default;

			EVENT_CLASS_METHODS(AppTick)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class KTN_API AppUpdateEvent : public Event
	{
		public:
			AppUpdateEvent() = default;

			EVENT_CLASS_METHODS(AppUpdate)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class KTN_API AppRenderEvent : public Event
	{
		public:
			AppRenderEvent() = default;

			EVENT_CLASS_METHODS(AppRender)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

} // namespace KTN