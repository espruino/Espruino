# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Tell Zephyr to look in our local project folder for custom boards
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(espruino)
set(CONF_FILE prj.conf)

target_sources(app PRIVATE main.c)
execute_process(COMMAND BOARD=NRF54L15DK make ../../Makefile cmake)
add_subdirectory(../../gen build )
