#include "epoll_excutor.hpp"
#include <sys/timerfd.h>

namespace CodeBase {
epoll_excutor::epoll_excutor() {
    init_epoll();
}
epoll_excutor::~epoll_excutor() {}

void epoll_excutor::init_epoll() {
    m_epoll_fd = epoll_create1(0);
    m_time_fd = timerfd_create(CLOCK_MONOTONIC, 0);
}

ErrorCode epoll_excutor::run() {
    epoll_event events[1024];

    auto nfds = epoll_wait(m_epoll_fd, events, totalCnt, -1);

    if (nfds > 0) {}

    if (nfds < 0) {}

    return Error::Success;
}

}// namespace CodeBase