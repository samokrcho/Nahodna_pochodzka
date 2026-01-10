add_executable(server server.c)

# linkovanie so shared/common kniznicou
target_link_libraries(server common pthread)
