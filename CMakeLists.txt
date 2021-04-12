cmake_minimum_required(VERSION 3.15)
project(project C)

set(CMAKE_C_STANDARD 99)

add_executable(project vovka.c test_vovka.c bn.h)