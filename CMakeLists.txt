cmake_minimum_required(VERSION 3.13)
project(TinyC)

set(CMAKE_CXX_STANDARD 11)

add_executable(TinyC TinyC.c SyntaxAnalysis.c LexicalAnalysis.c constvar.h )