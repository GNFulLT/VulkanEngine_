#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include "system_manager.h"
#include "../core/typedefs.h"

#include <boost/timer/timer.hpp>
#include <boost/chrono.hpp>

class TimerManager : public SystemManager
{
public:
	OBJECT_DEF(TimerManager,SystemManager)
	SINGLETON(TimerManager)
public:
	TimerManager() = default;

 	_INLINE_ int calc_fps()
	{
		static int frameCount = 0;
		frameCount++;
		if (m_lastDelta - m_lastTime > 1000.f)
		{
			m_frameCountInSecond = frameCount;
			frameCount = 0;
			m_lastTime += 1000.f;
			return m_frameCountInSecond;
		}
		return -1;
	}

	_INLINE_ int get_fps() const noexcept
	{
		return m_frameCountInSecond;
	}

	_INLINE_ double calculate_delta_time()
	{
		m_lastDelta = boost::chrono::duration_cast<boost::chrono::duration<double, boost::milli>>(boost::chrono::nanoseconds(m_timer.elapsed().wall)).count();
		m_delta = m_lastDelta - m_firstDelta;
		m_firstDelta = m_lastDelta;
		return m_delta;
	}

	_INLINE_ double get_last_delta()
	{
		return m_delta;
	}

private:
	boost::timer::cpu_timer m_timer;
	double m_delta = 0;
	double m_lastTime = 0;
	double m_lastDelta = 0;
	double m_firstDelta = 0;
	int m_frameCountInSecond = 0;
};


#endif // TIMER_MANAGER_H