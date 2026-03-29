/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-29.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * List — built-in mutable ordered collection.
 *
 * Registered as a Plain-callable class via bind_class<T>() so that
 * Plain code can create and manipulate lists:
 *
 *   set items, [list 1, 2, 3]
 *   print [items get 0]
 *   items append 42
 *   print [items length]
 */

#include "List.hpp"
#include <Plain/Runtime.hpp>

namespace plain::framework {

    /* Returns the element at <index>, or none if out of bounds. */
    Value List::get(int index) const {
        if(index < 0 || index >= static_cast<int>(items.size())) {
            return Value{};
        }
        return items[index];
    }

    /* Replaces the element at <index> with <value>.  No-op if out of bounds. */
    Value List::set(int index, Value value) {
        if(index < 0 || index >= static_cast<int>(items.size())) {
            return Value{};
        }
        items[index] = std::move(value);
        return Value{};
    }

    /* Returns the number of elements. */
    int List::length() const {
        return static_cast<int>(items.size());
    }

    /* Appends <value> to the end of the list. */
    void List::append(Value value) {
        items.push_back(std::move(value));
    }

    /* Removes the element at <index>.  No-op if out of bounds. */
    void List::remove(int index) {
        if(index < 0 || index >= static_cast<int>(items.size())) {
            return;
        }
        items.erase(items.begin() + index);
    }

    /* Registers the List class as a Plain-callable "list" type.
     * The constructor accepts variadic arguments as initial elements. */
    void List::bind(Runtime& runtime) {
        runtime.bind_class<List>("list")
            .constructor([](const Arguments& arguments) {
                auto list = std::make_shared<List>();
                list->items.reserve(arguments.size());
                for(const auto& argument : arguments) {
                    list->items.push_back(argument);
                }
                return list;
            })
            .method("get",    &List::get)
            .method("set",    &List::set)
            .method("length", &List::length)
            .method("append", &List::append)
            .method("remove", &List::remove);
    }

} /* namespace plain::framework */
