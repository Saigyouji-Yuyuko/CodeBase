#pragma once

#include "coroutine/listener.hpp"
#include "epoll_excutor.hpp"
#include "utils/error.hpp"

namespace CodeBase {

struct IOAwait {
    bool await_ready() const noexcept;
    void await_suspend(std::coroutine_handle<> handle) noexcept;
    void await_resume() const noexcept;
};

class TcpConnection : public ExcuteTaskBase {
public:
    TcpConnection(epoll_excutor *excutor, std::string_view ip, std::uint16_t port);
    TcpConnection(epoll_excutor *excutor, std::string_view addr);


    TcpConnection(const TcpConnection &) = delete;
    TcpConnection(TcpConnection &&) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;
    TcpConnection &operator=(TcpConnection &&) = delete;
    ~TcpConnection();

    Task<std::pair<std::uint64_t, ErrorCode>> read(char *, std::uint64_t);
    Task<std::pair<std::uint64_t, ErrorCode>> write(const char *, std::uint64_t);
    Task<ErrorCode>                           close();

private:
    epoll_excutor *excutor = nullptr;
};
static_assert(Streamer<TcpConnection>);

class TcpListener : public ExcuteTaskBase {
public:
    TcpListener();
    TcpListener(const TcpListener &) = delete;
    TcpListener(TcpListener &&) = delete;
    TcpListener &operator=(const TcpListener &) = delete;
    TcpListener &operator=(TcpListener &&) = delete;
    ~TcpListener();
    using Streamer = TcpConnection;
    Task<std::pair<TcpConnection, ErrorCode>> accept();
};

static_assert(Listener<TcpListener>);
}// namespace CodeBase