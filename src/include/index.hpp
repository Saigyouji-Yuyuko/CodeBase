#pragma once
#include "Storage.hpp"
#include "utils/context.hpp"


namespace CodeBase {

template<typename T>
concept ConstIndexIterator = requires(T t) {
                                 typename T::KeyType;
                                 typename T::ValueType;
                                 { t->key } -> std::same_as<const typename T::KeyType &>;
                                 { t->value } -> std::same_as<const typename T::ValueType &>;
                                 { t.operator++() } -> std::same_as<T>;
                                 { t.operator++(int{}) } -> std::same_as<T>;
                                 { t.operator--() } -> std::same_as<T>;
                                 { t.operator--(int{}) } -> std::same_as<T>;
                                 { t.operator<=>(T{}) } -> std::same_as<int>;
                             };

template<typename T>
concept IndexIterator = ConstIndexIterator<T> && requires(T t, const T &ct) {
                                                     { t->key } -> std::same_as<typename T::KeyType &>;
                                                     { t->value } -> std::same_as<typename T::ValueType &>;
                                                 };

template<typename T>
concept IndexInterface = requires(T t) {
                             typename T::KeyType;
                             typename T::ValueType;
                             IndexIterator<typename T::Iterator>;
                             ConstIndexIterator<typename T::ConstIterator>;

                             {
                                 t.Insert(typename T::Context{}, typename T::KeyType{}, typename T::ValueType{})
                                 } -> std::same_as<ErrorCode>;
                             { t.Compare(typename T::KeyType{}, typename T::KeyType{}) } -> std::same_as<bool>;
                             { t.Delete(typename T::Context{}, typename T::KeyType{}) } -> std::same_as<ErrorCode>;
                             {
                                 t.Find(typename T::Context{}, typename T::KeyType{})
                                 } -> std::same_as<typename T::ConstIterator>;
                             {
                                 t.Find(typename T::Context{}, typename T::KeyType{})
                                 } -> std::same_as<typename T::Iterator>;
                             {
                                 t.Update(typename T::Context{}, typename T::KeyType{}, typename T::ValueType{})
                                 } -> std::same_as<ErrorCode>;
                             { t.Begin(typename T::Context{}) } -> std::same_as<typename T::Iterator>;
                             { t.Begin(typename T::Context{}) } -> std::same_as<typename T::ConstIterator>;
                             { t.End(typename T::Context{}) } -> std::same_as<typename T::Iterator>;
                             { t.End(typename T::Context{}) } -> std::same_as<typename T::ConstIterator>;
                         };

template<typename T>
concept IndexWrap = requires(T t, const T &ct) {
                        IndexInterface<typename T::IndexType>;
                        { t.GetIndexe() } -> std::same_as<typename T::IndexType &>;
                        { ct.GetIndexe() } -> std::same_as<const typename T::IndexType &>;
                    };
}// namespace CodeBase