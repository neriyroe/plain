#
# Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
# Date     12/30/2013.
# Revision 09/01/2015.
#
# Copyright 2015 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Plain)
IF(PLAIN_DEBUGGING)
    SET(CMAKE_BUILD_TYPE Debug)
ELSE()
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Source/Framework/Evaluate.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Framework/Host/Version.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Runtime/Walk.c 
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Concat.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Algorithm/Hash.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Processing/Tokenize.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/System/Resize.c)
IF(PLAIN_DEBUGGING)
    ADD_DEFINITIONS(-DPLAIN_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
IF(PLAIN_EMBEDDED)
    ADD_LIBRARY(plain STATIC ${SOURCE})
ELSE()
    ADD_LIBRARY(plain SHARED ${SOURCE})
ENDIF()
