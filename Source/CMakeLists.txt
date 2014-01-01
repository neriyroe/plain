#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/30/2013,
# Revision 01/01/2013,
#
# Copyright 2013 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(pfvl)
SET(CMAKE_BUILD_TYPE Release)
SET(SOURCE
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
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING=1)
ENDIF()
INCLUDE_DIRECTORIES(../Include)
ADD_LIBRARY(pfvl SHARED ${SOURCE})
