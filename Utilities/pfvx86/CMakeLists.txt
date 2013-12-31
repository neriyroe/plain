#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 12/31/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

cmake_minimum_required(VERSION 2.8.3)
project(pfvx86)
set(CMAKE_BUILD_TYPE Release)
set(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Easy.c
    ${CMAKE_CURRENT_LIST_DIR}/Session.c
    ${CMAKE_CURRENT_LIST_DIR}/System/Load.c)
if(MOCOSEL_DEBUGGING)
    add_definitions(-DMOCOSEL_DEBUGGING=1)
endif()
include_directories(../../Include)
add_executable(pfvx86 ${SOURCE})
target_link_libraries(pfvx86 pfvl)
