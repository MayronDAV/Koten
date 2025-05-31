#pragma once
#include "Event.h"



namespace KTN
{
	class KTN_API JoystickConnectedEvent : public Event
	{
		public:
			JoystickConnectedEvent(int p_ID, const std::string& p_Name = "Unknown")
				: m_JoystickID(p_ID), m_Name(p_Name) {}

			std::string GetJoystickName() const { return m_Name; }
			int GetJoystickID() const { return m_JoystickID; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "JoystickConnectedEvent: " << m_Name << " (ID: " << m_JoystickID << ")";
				return ss.str();
			}

			EVENT_CLASS_CATEGORY(EventCategoryJoystick)
			EVENT_CLASS_METHODS(JoystickConnected)

		private:
			int m_JoystickID = -1;
			std::string m_Name;
	};

	class KTN_API JoystickDisconnectedEvent : public Event
	{
		public:
			JoystickDisconnectedEvent(int p_ID, const std::string& p_Name = "Unknown")
				: m_JoystickID(p_ID), m_Name(p_Name) {
			}

			std::string GetJoystickName() const { return m_Name; }
			int GetJoystickID() const { return m_JoystickID; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "JoystickDisconnectedEvent: " << m_Name << " (ID: " << m_JoystickID << ")";
				return ss.str();
			}

			EVENT_CLASS_CATEGORY(EventCategoryJoystick)
			EVENT_CLASS_METHODS(JoystickDisconnected)

		private:
			int m_JoystickID = -1;
			std::string m_Name;
	};

	class KTN_API GamepadEvent : public Event
	{
		public:
			inline int GetGamepadCode() const { return m_GamepadCode; }
			inline int GetJoystickID() const { return m_JoystickID; }

			EVENT_CLASS_CATEGORY(EventCategoryJoystick | EventCategoryInput)

		protected:
			explicit GamepadEvent(int p_JoystickID, int p_GamepadCode)
				: m_GamepadCode(p_GamepadCode), m_JoystickID(p_JoystickID) {}

			int m_GamepadCode;
			int m_JoystickID;
	};

	class KTN_API GamepadButtonPressedEvent : public GamepadEvent
	{
		public:
			GamepadButtonPressedEvent(int p_JoystickID, int p_GamepadCode, bool p_IsRepeat = false)
				: GamepadEvent(p_JoystickID, p_GamepadCode), m_IsRepeat(p_IsRepeat) {
			}

			bool IsRepeat() const { return m_IsRepeat; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "GamepadButtonPressedEvent: " << m_GamepadCode << " (repeat = " << m_IsRepeat << ")";
				return ss.str();
			}

			EVENT_CLASS_METHODS(GamepadButtonPressed)

		private:
			bool m_IsRepeat;
	};

	class KTN_API GamepadButtonReleasedEvent : public GamepadEvent
	{
		public:
			explicit GamepadButtonReleasedEvent(int p_JoystickID, int p_GamepadCode)
				: GamepadEvent(p_JoystickID, p_GamepadCode) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "GamepadButtonReleasedEvent: " << m_GamepadCode;
				return ss.str();
			}

			EVENT_CLASS_METHODS(GamepadButtonReleased)
	};

	class KTN_API GamepadAxisEvent : public GamepadEvent
	{
		public:
			GamepadAxisEvent(int p_JoystickID, int p_GamepadCode, float p_Value)
				: GamepadEvent(p_JoystickID, p_GamepadCode), m_AxisValue(p_Value) {}

			float GetAxisValue() const { return m_AxisValue; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "GamepadAxisEvent: " << m_GamepadCode << " (value = " << m_AxisValue << ")";
				return ss.str();
			}

			EVENT_CLASS_METHODS(GamepadAxis)

		private:
			float m_AxisValue;
	};

} // namespace KTN