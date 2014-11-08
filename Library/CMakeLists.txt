# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/30/2013,
# Revision 11/08/2014,
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
    ${CMAKE_CURRENT_LIST_DIR}/Source/Framework/Evaluate.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Framework/Host/Version.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Runtime/Walk.c 
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Concat.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Algorithm/Hash.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/Shared/Processing/Tokenize.c
    ${CMAKE_CURRENT_LIST_DIR}/Source/System/Resize.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
IF(MOCOSEL_EMBEDDED)
    ADD_LIBRARY(plain STATIC ${SOURCE})
ELSE()
    ADD_LIBRARY(plain SHARED ${SOURCE})
ENDIF()
