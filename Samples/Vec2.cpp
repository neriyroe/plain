/*
 * Sample: Vec2 class exposed to Plain via the C++ interface.
 *
 * Demonstrates the simplified builder API:
 *   .constructor<Args...>()   auto-converts Plain arguments to C++ types
 *   .method("name", &T::fn)  auto-wraps member function pointers
 *
 * Build & run:
 *   This file is not part of the main build. Compile manually:
 *
 *     cl /std:c++17 /I Include Samples/Vec2.cpp
 *        Library/Source/Framework/Plain.cpp -link plain.lib
 */

#include <Plain/Runtime.hpp>

#include <cmath>
#include <iostream>
#include <string>

struct Vec2 {
    double x, y;
    Vec2(double x, double y) : x(x), y(y) {}
    double length() const { return std::sqrt(x * x + y * y); }
    Vec2   add(const Vec2& o) const { return {x + o.x, y + o.y}; }
    std::string to_string() const {
        char buf[64];
        snprintf(buf, sizeof(buf), "Vec2(%g, %g)", x, y);
        return buf;
    }
};

int main() {
    plain::Runtime runtime;

    runtime.on_error([](const std::string& message) {
        std::cerr << message << '\n';
    });

    runtime.bind("print", [](const plain::Args& args) -> plain::Value {
        for(size_t i = 0; i < args.size(); i++) {
            if(i > 0) std::cout << ' ';
            std::cout << args[i].as_string();
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
