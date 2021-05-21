set(CMAKE_C_STANDARD 11)

# Outer packet
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

add_subdirectory(core)
add_subdirectory(model)

# include
include_directories(${CMAKE_SOURCE_DIR}/src)

# library
link_directories(${CMAKE_SOURCE_DIR}/bin)

# Compile Source Files
aux_source_directory(. DNS_SERVER_SRC)
add_executable(${PROJECT_NAME} ${DNS_SERVER_SRC})

# Link Libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} dnsRelayCore)
target_link_libraries(${PROJECT_NAME} dnsRelayModel)