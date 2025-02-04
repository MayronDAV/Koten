#pragma once

#include "Event.h"



namespace KTN
{

	class KTN_API KeyEvent : public Event
	{
		public:
			inline int GetKeyCode() const { return m_KeyCode; }

			EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

		protected:
			explicit KeyEvent(int p_KeyCode)
				: m_KeyCode(p_KeyCode) {}

			int m_KeyCode;
	};

	class KTN_API KeyPressedEvent : public KeyEvent
	{
		public:
			KeyPressedEvent(int p_KeyCode, bool p_IsRepeat = false)
				: KeyEvent(p_KeyCode), m_IsRepeat(p_IsRepeat) {}

			bool IsRepeat() const { return m_IsRepeat; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
				return ss.str();
			}

			EVENT_CLASS_METHODS(KeyPressed)

		private:
			bool m_IsRepeat;
	};

	class KTN_API KeyReleasedEvent : public KeyEvent
	{
		public:
			explicit KeyReleasedEvent(int p_KeyCode)
				: KeyEvent(p_KeyCode) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyReleasedEvent: " << m_KeyCode;
				return ss.str();
			}

			EVENT_CLASS_METHODS(KeyReleased)
	};

	class KTN_API KeyTypedEvent : public KeyEvent
	{
		public:
			explicit KeyTypedEvent(int p_KeyCode)
				: KeyEvent(p_KeyCode) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyTypedEvent: " << m_KeyCode;
				return ss.str();
			}

			EVENT_CLASS_METHODS(KeyTyped)
	};

} // namespace KTN