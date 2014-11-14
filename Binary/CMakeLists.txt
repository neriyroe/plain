# Author   Nerijus Ramanauskas <nr@mocosel.com>,
# Date     11/01/2014,
# Revision 11/14/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Dit)
IF(MOCOSEL_DEBUGGING)
    SET(CMAKE_BUILD_TYPE Debug)
ELSE()
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Runtime/Resolve.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Runtime/Report.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Run.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/GNU/Prompt.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Entry.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
ADD_EXECUTABLE(dit ${SOURCE})
TARGET_LINK_LIBRARIES(dit readline) 
TARGET_LINK_LIBRARIES(dit plain)
