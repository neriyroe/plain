/*
 * Tetris — Pure Plain game with SFML bindings.
 *
 * This file is a thin C++ host that wraps SFML as Plain-callable objects
 * and then loads Game.plain which contains the entire game.
 *
 * Build:  xmake build tetris
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

/* Piece data: 7 types x 4 rotations x 4 cells x 2 coords (x, y). */
static const int PIECE_DATA[7][4][4][2] = {
    /* I */ {{{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}}, {{0,2},{1,2},{2,2},{3,2}}, {{1,0},{1,1},{1,2},{1,3}}},
    /* O */ {{{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}},
    /* T */ {{{0,1},{1,1},{2,1},{1,0}}, {{1,0},{1,1},{1,2},{2,1}}, {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{1,1},{1,2},{0,1}}},
    /* S */ {{{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}}, {{1,1},{2,1},{0,2},{1,2}}, {{0,0},{0,1},{1,1},{1,2}}},
    /* Z */ {{{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}}, {{0,1},{1,1},{1,2},{2,2}}, {{1,0},{1,1},{0,1},{0,2}}},
    /* J */ {{{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}}, {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{1,2},{0,2}}},
    /* L */ {{{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}}, {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}}}
};

static const sf::Color PIECE_COLORS[8] = {
    sf::Color(40, 40, 40),       /* 0: empty cell background */
    sf::Color(0, 240, 240),      /* 1: I — cyan */
    sf::Color(240, 240, 0),      /* 2: O — yellow */
    sf::Color(160, 0, 240),      /* 3: T — purple */
    sf::Color(0, 240, 0),        /* 4: S — green */
    sf::Color(240, 0, 0),        /* 5: Z — red */
    sf::Color(0, 0, 240),        /* 6: J — blue */
    sf::Color(240, 160, 0)       /* 7: L — orange */
};

struct GameWindow {
    sf::RenderWindow window;
    sf::Font font;
    sf::Event last_event = {};
    bool has_event = false;
    bool font_loaded = false;

    GameWindow(int w, int h, const std::string& title)
        : window(sf::VideoMode(w, h), title) {
        window.setFramerateLimit(60);
        font_loaded = font.loadFromFile("C:/Windows/Fonts/consola.ttf");
        if(!font_loaded) font_loaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
        if(!font_loaded) font_loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
        if(!font_loaded) font_loaded = font.loadFromFile("/usr/share/fonts/TTF/DejaVuSansMono.ttf");
    }
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

static std::string key_name(sf::Keyboard::Key code) {
    switch(code) {
        case sf::Keyboard::Left:   return "left";
        case sf::Keyboard::Right:  return "right";
        case sf::Keyboard::Down:   return "down";
        case sf::Keyboard::Up:     return "up";
        case sf::Keyboard::Space:  return "space";
        case sf::Keyboard::Escape: return "escape";
        case sf::Keyboard::P:      return "p";
        case sf::Keyboard::R:      return "r";
        default:                   return "";
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

    runtime.bind("piece_cells", [&runtime](const plain::Args& a) -> plain::Value {
        int type = a.size() > 0 ? a[0].as_integer() : 0;
        int rot  = a.size() > 1 ? a[1].as_integer() : 0;
        if(type < 0 || type > 6) type = 0;
        rot = ((rot % 4) + 4) % 4;
        plain::Args items;
        for(int c = 0; c < 4; c++) {
            items.push_back(plain::Value(PIECE_DATA[type][rot][c][0]));
            items.push_back(plain::Value(PIECE_DATA[type][rot][c][1]));
        }
        auto l = std::make_shared<plain::detail::List>(std::move(items));
        return runtime.wrap(l);
    });

    runtime.bind("piece_color", [](const plain::Args& a) -> plain::Value {
        int type = a.size() > 0 ? a[0].as_integer() : 0;
        return plain::Value(type + 1);
    });

    runtime.bind_class<GameWindow>("Window",
        [](const plain::Args& a) {
            int w = a.size() > 0 ? a[0].as_integer() : 800;
            int h = a.size() > 1 ? a[1].as_integer() : 600;
            std::string title = a.size() > 2 ? a[2].as_string() : "Plain";
            return std::make_shared<GameWindow>(w, h, title);
        },
        {
            { "poll", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.window.isOpen()) return plain::Value(false);
                gw.has_event = gw.window.pollEvent(gw.last_event);
                return plain::Value(gw.has_event);
            }},
            { "event_type", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.has_event) return "none";
                switch(gw.last_event.type) {
                    case sf::Event::Closed:     return "closed";
                    case sf::Event::KeyPressed: return "key";
                    default:                    return "other";
                }
            }},
            { "event_key", [](GameWindow& gw, const plain::Args&) -> plain::Value {
                if(!gw.has_event || gw.last_event.type != sf::Event::KeyPressed) return "";
                return key_name(gw.last_event.key.code);
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
                if(a.size() < 5) return {};
                float x = static_cast<float>(a[0].as_real());
                float y = static_cast<float>(a[1].as_real());
                float w = static_cast<float>(a[2].as_real());
                float h = static_cast<float>(a[3].as_real());
                int ci  = a[4].as_integer();
                sf::Color color = (ci >= 0 && ci < 8) ? PIECE_COLORS[ci] : sf::Color(ci, ci, ci);
                if(a.size() >= 7) {
                    color = sf::Color(a[4].as_integer(), a[5].as_integer(), a[6].as_integer());
                }
                sf::RectangleShape rect({w, h});
                rect.setPosition(x, y);
                rect.setFillColor(color);
                gw.window.draw(rect);
                return {};
            }},
            { "draw_text", [](GameWindow& gw, const plain::Args& a) -> plain::Value {
                if(a.size() < 4 || !gw.font_loaded) return {};
                std::string text = a[0].as_string();
                float x    = static_cast<float>(a[1].as_real());
                float y    = static_cast<float>(a[2].as_real());
                int size   = a[3].as_integer();
                int r = a.size() > 4 ? a[4].as_integer() : 255;
                int g = a.size() > 5 ? a[5].as_integer() : 255;
                int b = a.size() > 6 ? a[6].as_integer() : 255;
                sf::Text txt(text, gw.font, size);
                txt.setPosition(x, y);
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
        std::cerr << "Failed to load game script: " << script_path << '\n';
        return 1;
    }

    runtime.run(source);
    return 0;
}
