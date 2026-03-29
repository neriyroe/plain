/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-22.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Prompt — Windows console implementation using standard I/O streams.
 */

#include "../Prompt.hpp"

#include <iostream>
#include <string>

namespace plain {

    int prompt(const std::string& identifier, InputHandler handler) {
        for(;;) {
            std::cout << identifier << std::flush;
            std::string line;
            if(!std::getline(std::cin, line)) break;  /* EOF or Ctrl+Z. */

            if(!line.empty()) {
                const int code = handler(line);
                if(code != 0) return code;
            }
        }
        return 0;
    }

} /* namespace plain */
