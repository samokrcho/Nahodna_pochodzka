add_executable(client client.c)

# linkovanie so shared/common kniznicou
target_link_libraries(client common pthread)
