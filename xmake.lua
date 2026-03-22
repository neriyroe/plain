--
-- Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
--

set_project("plain")
set_version("2015.3")

set_languages("c11", "cxx17")

option("debugging", { description = "Enable debug assertions and symbols.", default = false })
option("embedded",  { description = "Build plain as a shared (dynamic) library.",  default = false })
option("tetris",    { description = "Build the Tetris demo (requires SFML).",      default = false })

if is_plat("linux", "macosx") then
    add_requires("readline")
end

if has_config("tetris") then
    add_requires("sfml ~2.6", { configs = { graphics = true, window = true, system = true } })
end

-- Library.
target("plain")
    if has_config("embedded") then
        set_kind("shared")
    else
        set_kind("static")
    end
    add_files("Library/Source/**.c")
    add_files("Library/Source/**.cpp")
    add_includedirs("Include", { public = true })
    add_includedirs("Vendor",  { public = true })
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
    add_deps("plain")
    add_files("Inspector/Source/Main.cpp")
    add_files("Inspector/Source/Plain/Runtime/Report.c")
    if is_plat("linux", "macosx") then
        add_files("Inspector/Source/GNU/Prompt.c")
        add_packages("readline")
    else
        add_files("Inspector/Source/Windows/Prompt.c")
    end
    if has_config("debugging") then
        add_defines("PLAIN_DEBUGGING")
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("fastest")
    end

-- Tetris demo (opt-in: xmake config --tetris=y; xmake build tetris).
if has_config("tetris") then
target("tetris")
    set_kind("binary")
    add_deps("plain")
    add_files("Samples/Tetris/Main.cpp")
    add_packages("sfml")
    set_optimize("fastest")
end
