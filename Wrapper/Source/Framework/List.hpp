/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-29.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#pragma once

#include <Plain/Value.hpp>

#include <vector>

namespace plain {

    class Runtime;

    namespace framework {

        class List {
        public:
            std::vector<Value> items;

            List() = default;

            Value get(int index) const;
            Value set(int index, Value value);
            int   length() const;
            void  append(Value value);
            void  remove(int index);

            static void bind(Runtime& runtime);
        };

    } /* namespace framework */

} /* namespace plain */
