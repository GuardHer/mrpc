#include "src/net/timer.h"

namespace mrpc
{

Timer::Timer(int fd) : FdEvent(fd)
{
}

Timer::~Timer()
{
}

void Timer::addTimerEvent(TimerEvent::s_ptr event)
{
}

void Timer::delTimerEvent(TimerEvent::s_ptr event)
{
}


}// namespace mrpc