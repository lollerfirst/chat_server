cmake_minimum_required(VERSION 3.14)
project(chatserver C)

add_executable(chatserver ./server.c)
add_executable(chat ./client.c)

add_compile_options(-Wall -Wextra -pedantic -Werror -std=C99 -g)

target_link_libraries(chat PRIVATE pthread)
target_link_libraries(chatserver PRIVATE pthread)
