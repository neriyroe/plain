#
# Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
# Date     12/31/2013,
# Revision 01/09/2014,
#
# Copyright 2013 Nerijus Ramanauskas.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(Plain)
ADD_SUBDIRECTORY(Source)
IF(MOCOSEL_DEBUGGING)
    ADD_SUBDIRECTORY(Development)
ENDIF()
