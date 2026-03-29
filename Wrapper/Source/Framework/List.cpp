/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/29/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include "List.hpp"
#include <Plain/Runtime.hpp>

namespace plain::framework {

    Value List::get(int index) const {
        if(index < 0 || index >= static_cast<int>(items.size())) return Value{};
        return items[index];
    }

    Value List::set(int index, Value value) {
        if(index < 0 || index >= static_cast<int>(items.size())) return Value{};
        items[index] = std::move(value);
        return Value{};
    }

    int List::length() const {
        return static_cast<int>(items.size());
    }

    void List::append(Value value) {
        items.push_back(std::move(value));
    }

    void List::remove(int index) {
        if(index < 0 || index >= static_cast<int>(items.size())) return;
        items.erase(items.begin() + index);
    }

    void List::bind(Runtime& runtime) {
        runtime.bind_class<List>("list")
            .constructor([](const Arguments& arguments) {
                auto list = std::make_shared<List>();
                list->items.reserve(arguments.size());
                for(const auto& argument : arguments) list->items.push_back(argument);
                return list;
            })
            .method("get",    &List::get)
            .method("set",    &List::set)
            .method("length", &List::length)
            .method("append", &List::append)
            .method("remove", &List::remove);
    }

} /* namespace plain::framework */
