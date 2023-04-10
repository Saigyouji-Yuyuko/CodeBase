#pragma once
#include "Storage.hpp"

namespace CodeBase {

template<typename T>
concept ContextInterface = StorageWrap<T>;

}// namespace CodeBase