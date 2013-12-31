#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/30/2013,
# Revision 12/31/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

cmake_minimum_required(VERSION 2.8.3)
project(pfvl)
set(CMAKE_BUILD_TYPE Release)
set(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Auxiliary.c
    ${CMAKE_CURRENT_LIST_DIR}/Framework/Finalize.c
    ${CMAKE_CURRENT_LIST_DIR}/Framework/Run.c
    ${CMAKE_CURRENT_LIST_DIR}/Framework/Version.c
    ${CMAKE_CURRENT_LIST_DIR}/Runtime/Lookup.c
    ${CMAKE_CURRENT_LIST_DIR}/Runtime/Register.c
    ${CMAKE_CURRENT_LIST_DIR}/Runtime/Unregister.c
    ${CMAKE_CURRENT_LIST_DIR}/Runtime/Walk.c 
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Concat.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Reserve.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Algorithm/Hash.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Processing/Purge.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Processing/Tokenize.c
    ${CMAKE_CURRENT_LIST_DIR}/x86/Resize.c)
if(MOCOSEL_DEBUGGING)
    add_definitions(-DMOCOSEL_DEBUGGING=1)
endif()
include_directories(../Include)
add_library(pfvl SHARED ${SOURCE})
