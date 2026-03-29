/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-29.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Prompt — platform-agnostic interface for the interactive input loop.
 *
 * Implemented per platform:
 *   GNU/Prompt.cpp     — GNU readline (Linux / macOS).
 *   Windows/Prompt.cpp — standard I/O streams (Windows).
 */

#pragma once

#include <functional>
#include <string>

namespace plain {

    /* Callback: receives one line of input, returns 0 to continue or
     * non-zero to terminate the loop. */
    using InputHandler = std::function<int(const std::string&)>;

    /* Displays <identifier> as a prompt, reads lines, and calls <handler>
     * for each non-empty line.  Returns when <handler> returns non-zero or
     * on end-of-file.  Returns 0 on EOF, or the first non-zero handler code. */
    int prompt(const std::string& identifier, InputHandler handler);

} /* namespace plain */
