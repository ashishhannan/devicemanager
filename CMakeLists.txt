cmake_minimum_required(VERSION 3.16)
project(ev07b_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# libpqxx (Postgres). Make sure libpqxx and libpq are installed.
find_path(PQXX_INCLUDE_DIRS "pqxx/pqxx")
find_library(PQXX_LIBRARIES NAMES pqxx)
find_library(PQ_LIBRARIES NAMES pq)

include_directories(${Boost_INCLUDE_DIRS} ${PQXX_INCLUDE_DIRS})

add_executable(ev07b_server
    src/main.cpp
    src/server.hpp
    src/server.cpp
    src/session.hpp
    src/session.cpp
    src/protocol_parser.hpp
    src/protocol_parser.cpp
    src/db.hpp
    src/db.cpp
)

target_link_libraries(ev07b_server PRIVATE ${Boost_LIBRARIES} Threads::Threads ${PQXX_LIBRARIES} ${PQ_LIBRARIES})

