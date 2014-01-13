#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/30/2013,
# Revision 01/13/2014,
#
# Copyright 2014 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(plain)
SET(CMAKE_BUILD_TYPE Release)
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Framework/Run.c
    ${CMAKE_CURRENT_LIST_DIR}/Framework/Host/Version.c
    ${CMAKE_CURRENT_LIST_DIR}/Runtime/Walk.c 
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Concat.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Algorithm/Hash.c
    ${CMAKE_CURRENT_LIST_DIR}/Shared/Processing/Tokenize.c
    ${CMAKE_CURRENT_LIST_DIR}/x86/Resize.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING=1)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
IF(MOCOSEL_EMBEDDED)
    ADD_LIBRARY(plain STATIC $(SOURCE))
ELSE()
    ADD_LIBRARY(plain SHARED ${SOURCE})
ENDIF()
