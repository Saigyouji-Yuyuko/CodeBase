#pragma once
#include <concepts>
#include <string_view>

#include "error.hpp"

namespace CodeBase {
struct FileAttribute {};

template<typename T>
concept FileBase = requires(T a) {
                       { a.name() } noexcept -> std::same_as<std::string_view>;
                       { a.open() } -> std::same_as<ErrorCode>;
                       { a.close() } -> std::same_as<ErrorCode>;
                       { a.is_open() } -> std::same_as<bool>;
                       { a.attr() } -> std::same_as<std::pair<size_t, FileAttribute *>>;
                   };

template<typename T>
concept StreamReadOnlyFile = FileBase<T> && requires(T a, std::string_view str) {
                                                { a.read(str) } -> std::same_as<std::pair<size_t, ErrorCode>>;
                                            };

template<typename T>
concept StreamWriteOnlyFile = FileBase<T> && requires(T a, std::string_view str) {
                                                 { a.write(str) } -> std::same_as<std::pair<size_t, ErrorCode>>;
                                             };

template<typename T>
concept ReadOnlyFile = FileBase<T> && requires(T a, size_t offset, std::string_view str) {
                                          { a.read(offset, str) } -> std::same_as<std::pair<size_t, ErrorCode>>;
                                      };

template<typename T>
concept WritableFile = ReadOnlyFile<T> && requires(T a, size_t offset, const std::string_view &str) {
                                              { a.write(offset, str) } -> std::same_as<std::pair<size_t, ErrorCode>>;
                                          };

template<typename T>
concept Environment = requires(T a) {
                          typename T::WritableFileType;
                          typename T::ReadOnlyFileType;
                          typename T::StreamWriteOnlyFileType;
                          typename T::StreamReadOnlyFileType;

                          {
                              a.CreateWritableFile(std::string_view())
                              } -> std::same_as<std::pair<typename T::WritableFileType, ErrorCode>>;
                          {
                              a.CreateReadOnlyFile(std::string_view())
                              } -> std::same_as<std::pair<typename T::ReadOnlyFileType, ErrorCode>>;
                          {
                              a.CreateStreamWriteOnlyFile(std::string_view())
                              } -> std::same_as<std::pair<typename T::StreamWriteOnlyFileType, ErrorCode>>;
                          {
                              a.CreateStreamReadOnlyFile(std::string_view())
                              } -> std::same_as<std::pair<typename T::StreamReadOnlyFileType, ErrorCode>>;
                      };

struct ExampleEnvironment {};

}// namespace CodeBase