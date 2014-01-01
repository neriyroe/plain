#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/01/2014,
#
# Copyright 2013 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(pfvx86)
SET(CMAKE_BUILD_TYPE Release)
SET(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/Easy.c
    ${CMAKE_CURRENT_LIST_DIR}/Session.c
    ${CMAKE_CURRENT_LIST_DIR}/System/Load.c)
IF(MOCOSEL_DEBUGGING)
    ADD_DEFINITIONS(-DMOCOSEL_DEBUGGING=1)
ENDIF()
INCLUDE_DIRECTORIES(../../Include)
ADD_EXECUTABLE(pfvx86 ${SOURCE})
TARGET_LINK_LIBRARIES(pfvx86 pfvl)
