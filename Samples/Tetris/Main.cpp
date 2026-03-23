/*
 * Tetris host — pure SFML wrapper for Plain.
 *
 * This file knows NOTHING about Tetris. It wraps SFML primitives
 * (Window, Clock) as Plain-callable objects and runs a .plain script.
 *
 * Build:  xmake f --tetris=y; xmake build tetris
 * Run:    xmake run tetris
 */

#include <Plain/Plain.hpp>

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct GameWindow {
    sf::RenderWindow window;
    sf::Font font;
    sf::Event last_event = {};
    bool has_event = false;
    bool font_loaded = false;
};

struct GameClock {
    sf::Clock clock;
};

static std::string load_file(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        std::cerr << "Cannot open: " << path << '\n';
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::string sfml_key_name(sf::Keyboard::Key code) {
    switch(code) {
        case sf::Keyboard::A: return "a"; case sf::Keyboard::B: return "b";
        case sf::Keyboard::C: return "c"; case sf::Keyboard::D: return "d";
        case sf::Keyboard::E: return "e"; case sf::Keyboard::F: return "f";
        case sf::Keyboard::G: return "g"; case sf::Keyboard::H: return "h";
        case sf::Keyboard::I: return "i"; case sf::Keyboard::J: return "j";
        case sf::Keyboard::K: return "k"; case sf::Keyboard::L: return "l";
        case sf::Keyboard::M: return "m"; case sf::Keyboard::N: return "n";
        case sf::Keyboard::O: return "o"; case sf::Keyboard::P: return "p";
        case sf::Keyboard::Q: return "q"; case sf::Keyboard::R: return "r";
        case sf::Keyboard::S: return "s"; case sf::Keyboard::T: return "t";
        case sf::Keyboard::U: return "u"; case sf::Keyboard::V: return "v";
        case sf::Keyboard::W: return "w"; case sf::Keyboard::X: return "x";
        case sf::Keyboard::Y: return "y"; case sf::Keyboard::Z: return "z";
        case sf::Keyboard::Num0: return "0"; case sf::Keyboard::Num1: return "1";
        case sf::Keyboard::Num2: return "2"; case sf::Keyboard::Num3: return "3";
        case sf::Keyboard::Num4: return "4"; case sf::Keyboard::Num5: return "5";
        case sf::Keyboard::Num6: return "6"; case sf::Keyboard::Num7: return "7";
        case sf::Keyboard::Num8: return "8"; case sf::Keyboard::Num9: return "9";
        case sf::Keyboard::Escape: return "escape";
        case sf::Keyboard::Space:  return "space";
        case sf::Keyboard::Enter:  return "enter";
        case sf::Keyboard::Tab:    return "tab";
        case sf::Keyboard::Left:   return "left";
        case sf::Keyboard::Right:  return "right";
        case sf::Keyboard::Up:     return "up";
        case sf::Keyboard::Down:   return "down";
        case sf::Keyboard::LShift: case sf::Keyboard::RShift: return "shift";
        case sf::Keyboard::LControl: case sf::Keyboard::RControl: return "control";
        case sf::Keyboard::LAlt: case sf::Keyboard::RAlt: return "alt";
        case sf::Keyboard::BackSpace: return "backspace";
        case sf::Keyboard::Delete:    return "delete";
        default: return "";
    }
}

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    plain::Runtime runtime;

    runtime.on_error([](const std::string& msg) {
        std::cerr << "Error: " << msg << '\n';
    });

    runtime.bind("print", [](const plain::Args& args) -> plain::Value {
        for(size_t i = 0; i < args.size(); i++) {
            if(i > 0) std::cout << ' ';
            std::cout << args[i].as_string();
        }
        std::cout << '\n';
        return {};
    });

    runtime.bind("random", [](const plain::Args& a) -> plain::Value {
        int lo = a.size() > 0 ? a[0].as_integer() : 0;
        int hi = a.size() > 1 ? a[1].as_integer() : lo;
        if(hi < lo) std::swap(lo, hi);
        return plain::Value(lo + std::rand() % (hi - lo + 1));
    });

    runtime.bind_class<GameWindow>("Window",
        [](const plain::Args& a) {
            int w = a.size() > 0 ? a[0].as_integer() : 800;
            int h = a.size() > 1 ? a[1].as_integer() : 600;
            std::string title = a.size() > 2 ? a[2].as_string() : "Plain";
            auto gw = std::make_shared<GameWindow>();
            gw->window.create(sf::VideoMode(w, h), title);
            gw->window.setFramerateLimit(60);
            return gw;
        },
        {
            { "load_font", [](GameWindow& gw, const plain::Args& a) -> plain::Value {
                if(a.empty()) return plain::Value(false);
                gw.font_loaded = gw.font.loadFromFile(a[0].as_string());
                return plain::Value(gw.font_loaded);
            }},
            { "poll", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.window.isOpen()) return plain::Value(false);
                gw.has_event = gw.window.pollEvent(gw.last_event);
                return plain::Value(gw.has_event);
            }},
            { "event_type", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.has_event) return "none";
                switch(gw.last_event.type) {
                    case sf::Event::Closed:      return "closed";
                    case sf::Event::KeyPressed:  return "key_pressed";
                    case sf::Event::KeyReleased: return "key_released";
                    case sf::Event::Resized:     return "resized";
                    default:                     return "other";
                }
            }},
            { "event_key", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.has_event) return "";
                if(gw.last_event.type != sf::Event::KeyPressed &&
                   gw.last_event.type != sf::Event::KeyReleased) return "";
                return sfml_key_name(gw.last_event.key.code);
            }},
            { "close", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                gw.window.close();
                return {};
            }},
            { "closed", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                return plain::Value(!gw.window.isOpen());
            }},
            { "clear", [](GameWindow& gw, const plain::Args& a) -> plain::Value {
                int r = a.size() > 0 ? a[0].as_integer() : 0;
                int g = a.size() > 1 ? a[1].as_integer() : 0;
                int b = a.size() > 2 ? a[2].as_integer() : 0;
                gw.window.clear(sf::Color(r, g, b));
                return {};
            }},
            { "display", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                gw.window.display();
                return {};
            }},
            { "draw_rect", [](GameWindow& gw, const plain::Args& a) -> plain::Value {
                if(a.size() < 7) return {};
                float x = static_cast<float>(a[0].as_real());
                float y = static_cast<float>(a[1].as_real());
                float w = static_cast<float>(a[2].as_real());
                float h = static_cast<float>(a[3].as_real());
                sf::RectangleShape rect({w, h});
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(a[4].as_integer(), a[5].as_integer(), a[6].as_integer()));
                gw.window.draw(rect);
                return {};
            }},
            { "draw_text", [](GameWindow& gw, const plain::Args& a) -> plain::Value {
                if(a.size() < 4 || !gw.font_loaded) return {};
                sf::Text txt(a[0].as_string(), gw.font, a[3].as_integer());
                txt.setPosition(static_cast<float>(a[1].as_real()),
                                static_cast<float>(a[2].as_real()));
                int r = a.size() > 4 ? a[4].as_integer() : 255;
                int g = a.size() > 5 ? a[5].as_integer() : 255;
                int b = a.size() > 6 ? a[6].as_integer() : 255;
                txt.setFillColor(sf::Color(r, g, b));
                gw.window.draw(txt);
                return {};
            }}
        }
    );

    runtime.bind_class<GameClock>("Clock",
        [](const plain::Args&) { return std::make_shared<GameClock>(); },
        {
            { "restart", [](GameClock& gc, const plain::Args&) -> plain::Value {
                return plain::Value(static_cast<double>(gc.clock.restart().asSeconds()));
            }},
            { "elapsed", [](GameClock& gc, const plain::Args&) -> plain::Value {
                return plain::Value(static_cast<double>(gc.clock.getElapsedTime().asSeconds()));
            }}
        }
    );

    std::string script_path = "Samples/Tetris/Game.plain";
    if(argc > 1) script_path = argv[1];

    std::string source = load_file(script_path);
    if(source.empty()) {
        std::cerr << "Failed to load script: " << script_path << '\n';
        return 1;
    }

    runtime.run(source);
    return 0;
}
