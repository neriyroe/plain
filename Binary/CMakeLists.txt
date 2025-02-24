#
# Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
# Date     11/01/2014.
# Revision 02/21/2016.
#
# Copyright 2016 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Inspector)
IF(PLAIN_DEBUGGING)
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
IF(PLAIN_DEBUGGING)
    ADD_DEFINITIONS(-DPLAIN_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
ADD_EXECUTABLE(Inspector ${SOURCE})
TARGET_LINK_LIBRARIES(Inspector readline) 
TARGET_LINK_LIBRARIES(Inspector plain)
