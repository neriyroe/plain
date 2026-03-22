--
-- Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
--

set_project("plain")
set_version("2015.3")

option("debugging", { description = "Enable debug assertions and symbols.", default = false })
option("embedded",  { description = "Build plain as a shared (dynamic) library.",  default = false })

if is_plat("linux", "macosx") then
    add_requires("readline")
end

-- Library.
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

-- Inspector (REPL).
target("inspector")
    set_kind("binary")
    add_deps("plain")
    add_files("Inspector/Source/Application.c")
    add_files("Inspector/Source/Plain/**.c")
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
