#pragma once
#include <coroutine>
#include <limits>
#include <memory>
#include <thread>
#include <cstdint>
#include <functional>
#include <list>
#include <type_traits>
#include <boost/stacktrace.hpp>
namespace CodeBase {

    struct Scheduler;

    struct TaskBase
    { 
        enum class TaskType
        {
            None,Coroutine,Function,Polling
        };
        TaskBase(TaskType);
        TaskType type = TaskType::None;
        uint64_t id = std::numeric_limits<uint64_t>::min();
        void Run(Scheduler*);
    };

    struct PollingTask :public TaskBase
    {
        PollingTask(const std::function<void(PollingTask*)> &);
        PollingTask(std::function<void(PollingTask*)> &&);

        PollingTask(const PollingTask&) = default;
        PollingTask(PollingTask&&) = default;

        PollingTask& operator=(const PollingTask&) = default;
        PollingTask& operator=(PollingTask&&) = default;
        
        ~PollingTask() = default;
        
        PollingTask* next = nullptr;
        std::function<void(PollingTask*)> f = nullptr;
        Scheduler* sche = nullptr;
        bool deleted = false;
    };

    struct Scheduler 
    {
        virtual void addTask(TaskBase*) = 0;
        virtual void addPollingTask(PollingTask*) = 0;
        virtual TaskBase* nowTask()const noexcept = 0;
        virtual void stop() = 0;
        virtual ~Scheduler() = 0;
    };
    
    struct FunctionTask final : public TaskBase
    {
        FunctionTask(const std::function<void()> &);
        FunctionTask(std::function<void()> &&);

        FunctionTask(const FunctionTask&) = default;
        FunctionTask(FunctionTask&&) = default;

        FunctionTask& operator=(const FunctionTask&) = default;
        FunctionTask& operator=(FunctionTask&&) = default;
        ~FunctionTask() = default;
        std::function<void()> f = nullptr;
    };

    struct TestScheduler final: public Scheduler,public std::enable_shared_from_this<TestScheduler>
    {
    public:
        TestScheduler();
        virtual void addTask(TaskBase*) override;
        virtual void addPollingTask(PollingTask*) override;
        virtual TaskBase* nowTask()const noexcept override;
        virtual void stop() override;
        void start();
    private:
        virtual ~TestScheduler() override;
        void loop();
        
        std::unique_ptr<std::thread> th;
        std::list<TaskBase*> lists;
        PollingTask* list = nullptr;
        TaskBase* nowTask_ = nullptr;
        bool stopd = false;
    };
}