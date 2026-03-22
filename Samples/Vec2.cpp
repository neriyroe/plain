/*
 * Sample: Vec2 class exposed to Plain via the C++ interface.
 *
 * Demonstrates:
 *   - bind_class<T>  registers a C++ class (constructor + methods)
 *   - wrap<T>        wraps a C++ shared_ptr as a Plain value
 *   - as_object<T>   extracts the C++ object from a Plain value
 *   - call()         calls a Plain procedure from C++
 *
 * Build & run:
 *   This file is not part of the main build. Compile manually:
 *
 *     cl /std:c++17 /I Include Samples/Vec2.cpp
 *        Library/Source/Framework/Plain.cpp -link plain.lib
 */

#include <Plain/Plain.hpp>

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

    runtime.bind_class<Vec2>("Vec2",
        [](const plain::Args& a) {
            double x = a.size() > 0 ? a[0].as_real() : 0.0;
            double y = a.size() > 1 ? a[1].as_real() : 0.0;
            return std::make_shared<Vec2>(x, y);
        },
        {
            { "length", [](Vec2& v, const plain::Args&) -> plain::Value {
                return plain::Value(v.length());
            }},
            { "add", [&runtime](Vec2& v, const plain::Args& a) -> plain::Value {
                auto& other = runtime.as_object<Vec2>(a[0]);
                return runtime.wrap(std::make_shared<Vec2>(v.add(other)));
            }},
            { "str", [](Vec2& v, const plain::Args&) -> plain::Value {
                return v.to_string();
            }}
        }
    );

    runtime.run(R"(
        a = [Vec2 3, 4];
        print [a "str"];
        print [a "length"];

        b = [Vec2 1, 2];
        c = [a "add" b];
        print [c "str"];

        greet = [procedure {name} { print [join "Hello, " name "!"] }];
        greet "World"
    )");

    return 0;
}
