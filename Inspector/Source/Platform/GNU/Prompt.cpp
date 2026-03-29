/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2014-11-01.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Prompt — GNU readline implementation (Linux / macOS).
 */

#include "../Prompt.hpp"

#include <readline/history.h>
#include <readline/readline.h>

#include <cstdlib>

namespace plain {

    int prompt(const std::string& identifier, InputHandler handler) {
        for(;;) {
            char* line = readline(identifier.c_str());
            if(line == nullptr) break;  /* EOF or Ctrl+D. */

            if(*line != '\0') {
                add_history(line);
                const int code = handler(line);
                std::free(line);
                if(code != 0) return code;
            } else {
                std::free(line);
            }
        }
        return 0;
    }

} /* namespace plain */
