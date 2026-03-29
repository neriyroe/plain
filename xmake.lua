--
-- Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
-- Date     2026-03-29
-- Revision 2026-03-29
--
-- Copyright 2026 Nerijus Ramanauskas.
--

set_project("plain")
set_version("2015.3")

set_languages("c11", "cxx17")

option("debugging", { description = "Enable debug assertions and symbols.", default = false })
option("embedded",  { description = "Build plain as a shared (dynamic) library.",  default = false })

if is_plat("linux", "macosx") then
    add_requires("readline")
end

-- Core C library.
target("plain")
    if has_config("embedded") then
        set_kind("shared")
    else
        set_kind("static")
    end
    add_files("Library/Source/**.c")
    add_includedirs("Include", { public = true })
    add_includedirs("Vendor",  { public = true })
    if has_config("debugging") then
        add_defines("PLAIN_DEBUGGING")
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("fastest")
    end

-- C++ wrapper library.
target("plain_wrapper")
    set_kind("static")
    add_deps("plain")
    add_files("Wrapper/Source/**.cpp")
    add_includedirs("Wrapper/Include", { public = true })
    if has_config("debugging") then
        add_defines("PLAIN_DEBUGGING")
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("fastest")
    end

-- Inspector (REPL).
target("inspector")
    set_kind("binary")
    add_deps("plain_wrapper")
    add_files("Inspector/Source/Main.cpp")
    if is_plat("linux", "macosx") then
        add_files("Inspector/Source/Platform/GNU/Prompt.cpp")
        add_packages("readline")
    else
        add_files("Inspector/Source/Platform/Windows/Prompt.cpp")
    end
    if has_config("debugging") then
        add_defines("PLAIN_DEBUGGING")
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("fastest")
    end
