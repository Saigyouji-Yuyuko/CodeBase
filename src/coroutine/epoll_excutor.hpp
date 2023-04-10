#pragma once
#include "coroutine/scheduler.hpp"
#include "utils/error.hpp"
#include <set>
#include <sys/epoll.h>
#include <sys/socket.h>
namespace CodeBase {
class epoll_excutor {
public:
    epoll_excutor();
    ~epoll_excutor();

    ErrorCode run();

    ErrorCode AddTask();
    ErrorCode RemoveTask();

private:
    void init_epoll();
    int  m_epoll_fd;
    int  m_time_fd;

    size_t                 totalCnt = 0;
    std::set<TimeOutEvent> timeout_events;
};
}// namespace CodeBase