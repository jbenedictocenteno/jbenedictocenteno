# The three lines every ESP-IDF project starts with, in this exact order.
#
#   1. cmake_minimum_required  — plain CMake.
#   2. include(project.cmake)  — pulls in ESP-IDF, which *replaces* CMake's
#                                own project() command with its own macro.
#   3. project(<name>)         — so this call is ESP-IDF's macro, not CMake's.
#                                It is what triggers idf_build_process().
#
# Swap 2 and 3 and you get a plain CMake project with no ESP-IDF in it.

cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(idf_tour)
