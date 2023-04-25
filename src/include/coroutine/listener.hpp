#pragma once

#include "Task.hpp"
#include "scheduler.hpp"
#include "utils/error.hpp"
#include <cstdint>

namespace CodeBase {

template<typename T>
concept Streamer = requires(T a) {
                       { a.read((char *){}, uint64_t{}) } -> std::same_as<Task<std::pair<uint64_t, ErrorCode>>>;
                       { a.write((const char *){}, uint64_t{}) } -> std::same_as<Task<std::pair<uint64_t, ErrorCode>>>;
                       { a.close() } -> std::same_as<Task<ErrorCode>>;
                   };

template<typename T>
concept Listener = requires(T a) {
                       Streamer<typename T::Streamer>;
                       { a.accept() } -> std::same_as<Task<std::pair<typename T::Streamer, ErrorCode>>>;
                   };

}// namespace CodeBase