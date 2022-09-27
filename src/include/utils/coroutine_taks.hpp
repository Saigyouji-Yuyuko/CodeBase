#pragma once
#include "utils/scheduler.hpp"
#include <coroutine>

namespace CodeBase 
{
    struct co_task_base : public TaskBase
    {
        void Do();
        virtual ~co_task_base();
        std::coroutine_handle<> h = nullptr;
        Scheduler* sche = nullptr;
    };

    template<typename T>
    struct co_promise;
    

    template<typename T = void>
    struct co_task :public co_task_base 
    {
        using promise_type = co_promise<T>;
        friend promise_type;
        bool await_ready()const noexcept{return isdone;}
        void await_suspend(std::coroutine_handle<> handle);
        T await_resume();
        ~co_task();
    private:
        void mark_done(T v)
        {
            result = std::move(v);
            mark_done();
        }
        void mark_done()
        {
            this->isdone = true;
        }


        Scheduler* scheduler = nullptr;
        bool isdone = false;
        T result;
        promise_type* promise = nullptr;
        std::coroutine_handle<> handle = nullptr;

        #ifdef COROUTINE_FRAME
        std::vector<boost::stacktrace::frame> frame;
        #endif
    };
    template<typename T>
    struct co_promise_base
    {
        using task_type = co_task<T>;
        friend task_type;

        task_type get_return_object();
        std::suspend_never initial_suspend()noexcept;
        std::suspend_never final_suspend() noexcept;
        void unhandled_exception();
    private:
        task_type* Task = nullptr;
    };

    template<typename T>
    struct co_promise : public co_promise_base<T>
    {    
        void return_value(T v);
    };

    template<>
    struct co_promise<void>
    {    
        void return_void();
    };

}