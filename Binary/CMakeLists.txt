# Author   Nerijus Ramanauskas <nr@mocosel.com>,
# Date     11/01/2014,
# Revision 03/10/2015,
#
# Copyright 2015 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Mocosel_Prompt_GNU)
IF(MOCOSEL_DEBUGGING)
    SET(CMAKE_BUILD_TYPE Debug)
ELSE()
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Runtime/Resolve.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Runtime/Report.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Plain/Evaluate.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/GNU/Prompt.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Application.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
ADD_EXECUTABLE(mocosel_prompt_gnu ${SOURCE})
TARGET_LINK_LIBRARIES(mocosel_prompt_gnu readline) 
TARGET_LINK_LIBRARIES(mocosel_prompt_gnu plain)
