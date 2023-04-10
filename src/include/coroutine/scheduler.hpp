
#include <thread>
namespace CodeBase {

struct schedulerTask {
    enum class schedulerTaskType {
        Invalid,
        IOWait,
        TimeWait,
        ReScheduler,
    };
};

class SingleThreadScheduler {
public:
    SingleThreadScheduler();
    ~SingleThreadScheduler();

    void run();

private:
    std::thread th;
};

enum IOEventType {
    Read = 1,
    Write = 2,
    TimeWait = 4,
};

struct TimeOutEvent {
    uint64_t             timeStamp;
    void                *data;
    std::strong_ordering operator<=>(const TimeOutEvent &other) const;
};


}// namespace CodeBase