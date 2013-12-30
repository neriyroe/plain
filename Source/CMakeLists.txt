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
    ${CMAKE_SOURCE_DIR}/Auxiliary.c
    ${CMAKE_SOURCE_DIR}/Framework/Finalize.c
    ${CMAKE_SOURCE_DIR}/Framework/Run.c
    ${CMAKE_SOURCE_DIR}/Framework/Version.c
    ${CMAKE_SOURCE_DIR}/Runtime/Lookup.c
    ${CMAKE_SOURCE_DIR}/Runtime/Register.c
    ${CMAKE_SOURCE_DIR}/Runtime/Unregister.c
    ${CMAKE_SOURCE_DIR}/Runtime/Walk.c 
    ${CMAKE_SOURCE_DIR}/Shared/Concat.c
    ${CMAKE_SOURCE_DIR}/Shared/Reserve.c
    ${CMAKE_SOURCE_DIR}/Shared/Algorithm/Hash.c
    ${CMAKE_SOURCE_DIR}/Shared/Processing/Purge.c
    ${CMAKE_SOURCE_DIR}/Shared/Processing/Tokenize.c
    ${CMAKE_SOURCE_DIR}/x86/Resize.c)
if(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DNDEBUG=1 -DMOCOSEL_DEBUGGING=1)
endif()
include_directories(../Include)
add_library(Plain SHARED ${SOURCE})
if(UNIX)
    set_target_properties(Plain PROPERTIES PREFIX "")
    set_target_properties(Plain PROPERTIES SUFFIX ".so")
endif()
