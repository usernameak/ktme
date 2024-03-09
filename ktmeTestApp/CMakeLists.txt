cmake_minimum_required(VERSION 3.17)

add_executable(ktmeTestApp main.c)
target_link_libraries(ktmeTestApp PRIVATE ktme)
