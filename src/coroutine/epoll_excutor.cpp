#include "epoll_excutor.hpp"
#include "utils/utils.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/timerfd.h>

namespace CodeBase {
epoll_excutor::epoll_excutor() {
    init_epoll();
}
epoll_excutor::~epoll_excutor() {}

void epoll_excutor::init_epoll() {
    m_epoll_fd = epoll_create1(0);
    m_time_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    epoll_event event;
    event.data.ptr = nullptr;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_time_fd, &event) < 0) {
        perror("epoll_ctl");
        assert(false);
    }
}

itimerspec transFromNanoSecond(uint64_t nano) {
    itimerspec time = {0, 0};
    time.it_value.tv_sec = nano / 1000000000;
    time.it_value.tv_nsec = nano % 1000000000;
    return time;
}

ErrorCode epoll_excutor::run() {

    {
        auto nowtime = now();
        for (auto it = timeout_events.begin(); it != timeout_events.end();) {
            if ((*it)->timeStamp > nowtime) {
                break;
            }
            (*it)->callback(*it);
            it = timeout_events.erase(it);
        }

        itimerspec newtime = {0, 0, 0, 0};
        if (this->timeout_events.size() > 0) {
            newtime = transFromNanoSecond((*this->timeout_events.begin())->timeStamp);
        }

        if (std::memcmp(&newtime, &lastTime, sizeof(itimerspec)) != 0) {
            timerfd_settime(m_time_fd, TFD_TIMER_ABSTIME, &newtime, nullptr);
            lastTime = newtime;
        }
    }

    auto nfds = epoll_wait(m_epoll_fd, events.data(), events.size(), -1);

    if (nfds > 0) {
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.ptr != nullptr) {
                auto taskbaseptr = static_cast<ExcuteTaskBase *>(events[i].data.ptr);
                taskbaseptr->excute();
            }
        }
    }

    auto nowtime = now();
    for (auto it = timeout_events.begin(); it != timeout_events.end();) {
        if ((*it)->timeStamp > nowtime) {
            break;
        }
        (*it)->callback(*it);
        it = timeout_events.erase(it);
    }

    if (nfds < 0) {
        perror("epoll_wait");
        return Error::Fail;
    }

    return Error::Success;
}

uint32_t transEpollEvent(uint64_t taskType) {
    uint32_t event = 0;
    if (taskType & ExcuteTaskBase::IOEventType::Read)
        event |= EPOLLIN;
    if (taskType & ExcuteTaskBase::IOEventType::Write)
        event |= EPOLLOUT;
    return event;
}

ErrorCode epoll_excutor::AddTask(ExcuteTaskBase *task) {
    epoll_event event;
    event.data.ptr = task;
    event.events = transEpollEvent(task->type) | EPOLLET;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, task->fd, &event) < 0) {
        perror("epoll_ctl");
        return Error::Fail;
    }
    ++totalCnt;
    return Error::Success;
}
ErrorCode epoll_excutor::RemoveTask(ExcuteTaskBase *task) {
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, task->fd, nullptr) < 0) {
        perror("epoll_ctl");
        return Error::Fail;
    }
    --totalCnt;
    return Error::Success;
}

ErrorCode epoll_excutor::ModifyTask(ExcuteTaskBase *task) {
    epoll_event event;
    event.data.ptr = task;
    event.events = transEpollEvent(task->type) | EPOLLET;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, task->fd, &event) < 0) {
        perror("epoll_ctl");
        return Error::Fail;
    }
    return Error::Success;
}

ErrorCode epoll_excutor::AddTimeOutEvent(TimeOutEvent *event) {
    timeout_events.insert(event);
    ++totalCnt;
    return Error::Success;
}
ErrorCode epoll_excutor::RemoveTimeOutEvent(TimeOutEvent *event) {
    timeout_events.erase(event);
    --totalCnt;
    return Error::Success;
}
}// namespace CodeBase