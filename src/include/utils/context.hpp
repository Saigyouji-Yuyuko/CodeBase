#pragma once
#include "Storage.hpp"

namespace CodeBase {

class IContext {
public:
    virtual ~IContext() = default;

    IStorage *GetStorage();
};

}// namespace CodeBase