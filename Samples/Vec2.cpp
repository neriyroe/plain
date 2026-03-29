/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-29.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Sample: Vec2 class exposed to Plain via the C++ interface.
 *
 * Demonstrates the simplified builder API:
 *   .constructor<Arguments...>()  auto-converts Plain arguments to C++ types
 *   .method("name", &T::fn)       auto-wraps member function pointers
 *
 * Build & run:
 *   This file is not part of the main build. Compile manually:
 *
 *     cl /std:c++17 /I Include /I Wrapper/Include Samples/Vec2.cpp -link plain_wrapper.lib plain.lib
 */

#include <Plain/Runtime.hpp>

#include <cmath>
#include <iostream>
#include <string>

struct Vec2 {
    double x, y;
    Vec2(double x, double y) : x(x), y(y) {}
    double length() const { return std::sqrt(x * x + y * y); }
    Vec2   add(const Vec2& other) const { return {x + other.x, y + other.y}; }
    std::string to_string() const {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Vec2(%g, %g)", x, y);
        return buffer;
    }
};

int main() {
    plain::Runtime runtime;

    runtime.on_error([](const std::string& message) {
        std::cerr << message << '\n';
    });

    runtime.bind("print", [](const plain::Arguments& arguments) -> plain::Value {
        for(size_t index = 0; index < arguments.size(); index++) {
            if(index > 0) std::cout << ' ';
            std::cout << arguments[index].as_string();
        }
        std::cout << '\n';
        return {};
    });

    runtime.bind_class<Vec2>("Vec2")
        .constructor<double, double>()
        .method("length", &Vec2::length)
        .method("add",    &Vec2::add)
        .method("str",    &Vec2::to_string);

    runtime.run(R"(
        set a, [Vec2 3, 4];
        print [a str];
        print [a length];

        set b, [Vec2 1, 2];
        set c, [+ a b];
        print [c str];

        set greet, [define {name} { print "Hello, {name}!" }];
        greet "World"
    )");

    return 0;
}
