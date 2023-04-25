#pragma once
#include "coroutine/scheduler.hpp"
#include "utils/error.hpp"
#include <set>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
namespace CodeBase {

struct pointTimeOutEvent {
    bool operator()(const TimeOutEvent *a, const TimeOutEvent *b) const {
        return a->timeStamp < b->timeStamp;
    }
};

class epoll_excutor {
public:
    epoll_excutor();
    ~epoll_excutor();

    ErrorCode run();

    ErrorCode AddTask(ExcuteTaskBase *task);
    ErrorCode RemoveTask(ExcuteTaskBase *task);
    ErrorCode ModifyTask(ExcuteTaskBase *task);

    ErrorCode AddTimeOutEvent(TimeOutEvent *event);
    ErrorCode RemoveTimeOutEvent(TimeOutEvent *event);

private:
    void       init_epoll();
    int        m_epoll_fd;
    int        m_time_fd;
    itimerspec lastTime = {0, 0, 0, 0};


    size_t                                      totalCnt = 0;
    std::vector<epoll_event>                    events;
    std::set<TimeOutEvent *, pointTimeOutEvent> timeout_events;
};
}// namespace CodeBase