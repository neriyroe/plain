# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     11/01/2014,
# Revision 11/01/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Plain)
IF(MOCOSEL_DEBUGGING)
    SET(CMAKE_BUILD_TYPE Debug)
ELSE()
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Source/Mocosel/Environment/Layer/Resolve.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Mocosel/Environment/Report.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Mocosel/Run.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/POSIX/Prompt.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Entry.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
ADD_EXECUTABLE(plain ${SOURCE})
TARGET_LINK_LIBRARIES(plain plain)
