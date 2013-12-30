#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/30/2013,
# Revision 12/30/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

cmake_minimum_required(VERSION 2.8)
project(Plain)
set(CMAKE_BUILD_TYPE Release)
set(SOURCE
    Auxiliary.c
    Framework/Finalize.c
    Framework/Run.c
    Framework/Version.c
    Runtime/Lookup.c
    Runtime/Register.c
    Runtime/Unregister.c
    Runtime/Walk.c 
    Shared/Concat.c
    Shared/Reserve.c
    Shared/Algorithm/Hash.c
    Shared/Processing/Purge.c
    Shared/Processing/Tokenize.c
    x86/Resize.c)
if(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DNDEBUG=1 -DMOCOSEL_DEBUGGING=1)
endif()
include_directories(../Include)
add_library(Plain SHARED ${SOURCE})
if(UNIX)
    set_target_properties(Plain PROPERTIES PREFIX "")
    set_target_properties(Plain PROPERTIES SUFFIX ".so")
endif()
