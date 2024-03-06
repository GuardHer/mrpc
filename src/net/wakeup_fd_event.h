#ifndef MRPC_NET_WAKEUP_FDEVENT_H
#define MRPC_NET_WAKEUP_FDEVENT_H


#include "src/net/fd_event.h"


namespace mrpc
{
class WakeUpFdEvent : public FdEvent
{
public:
    WakeUpFdEvent(int fd);
    ~WakeUpFdEvent();

    void wakeup();

private:
};
}// namespace mrpc

#endif//MRPC_NET_WAKEUP_FDEVENT_H