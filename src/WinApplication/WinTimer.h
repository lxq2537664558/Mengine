#	pragma once

#	include "Interface/ApplicationInterface.h"

#	include "WindowsLayer/WindowsIncluder.h"

namespace Menge
{
	class WinTimer 
		: public TimerInterface
	{
	public:
		WinTimer();

	public:
        void initialize() override;
		void reset() override;
		float getDeltaTime() const override;

		uint64_t getMilliseconds() override;
		uint64_t getMicroseconds() override;
		uint64_t getMillisecondsCPU() override;
		uint64_t getMicrosecondsCPU() override;

	private:
		DWORD m_timerMask;
		DWORD m_startTick;
		LONGLONG m_lastTime;
		mutable LARGE_INTEGER m_lastdt;
		LARGE_INTEGER m_startTime;
		LARGE_INTEGER m_frequency;
	};
};
