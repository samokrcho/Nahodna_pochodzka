# common kniznica
add_library(common STATIC
    walker.c
    world.c
    simulation.c
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
