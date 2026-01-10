cmake_minimum_required(VERSION 3.10)
project(random_walker C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -Wextra -pthread")

# pridaj common ako subprojekt
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
