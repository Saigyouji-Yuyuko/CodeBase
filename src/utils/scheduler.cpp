#include "utils/scheduler.hpp"
#include "utils/coroutine_taks.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
namespace CodeBase
{
    void TaskBase::Run(Scheduler* scheduler)
    {
        switch(this->type)
        {
        case TaskType::Coroutine:
        {
            auto tmp = static_cast<co_task_base*>(this);
            tmp->Do();
        }

        case TaskType::Function:
        {
            auto tmp = static_cast<FunctionTask*>(this);
            tmp->f();
            delete tmp;
        }
        default:
            assert(false);
        }
    }
    TaskBase::TaskBase(TaskType t):type(t){}


    PollingTask::PollingTask(const std::function<void(PollingTask*)> &f):TaskBase(TaskBase::TaskType::Polling),f{f}{}
    PollingTask::PollingTask(std::function<void(PollingTask*)> &&f):TaskBase(TaskBase::TaskType::Polling),f{std::move(f)}{}
    FunctionTask::FunctionTask(const std::function<void()> &f):TaskBase(TaskBase::TaskType::Function),f{f}{}
    FunctionTask::FunctionTask(std::function<void()> &&f):TaskBase(TaskBase::TaskType::Function),f{std::move(f)}{}


    void TestScheduler::addTask(TaskBase* task) 
    {
        this->lists.push_back(task);
    }
    void TestScheduler::addPollingTask(PollingTask* polltask) 
    {
        polltask->sche = this;
        polltask->next = this->list;
        this->list = polltask;
    }
    TaskBase* TestScheduler::nowTask()const noexcept 
    {
        return nowTask_;
    }
    void TestScheduler::stop() 
    {
        this->stopd = true;
    }

    TestScheduler::~TestScheduler(){}

    void TestScheduler::loop()
    {
        while(this->stopd)
        {
            auto tmp = std::move(this->lists);
            for(auto task: tmp)
            {
                this->nowTask_ = task;
                task->Run(this);
            }
            while(this->list != nullptr){
                this->nowTask_ = this->list;
                this->list->Run(this);
                if(this->list->deleted){
                    auto tmp = this->list->next;
                    delete this->list;
                    this->list = tmp;
                }
            }
            for(auto task = this->list;task->next != nullptr;task = task->next)
            {
                this->nowTask_ = task->next;
                task->next->Run(this);
                if(task->next->deleted){
                    auto tmp = task->next;
                    task->next = task->next->next;
                    delete tmp;
                }
            }
            this->nowTask_ = nullptr;
        }
    }

    void TestScheduler::start()
    {
        this->th = std::make_unique<std::thread>([t = this->shared_from_this()]{t->loop();});
    }
}