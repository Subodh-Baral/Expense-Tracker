cmake_minimum_required(VERSION 3.15)
project(ExpenseTracker)

set(CMAKE_CXX_STANDARD 17)

find_package(unofficial-argon2 CONFIG REQUIRED)

add_executable(app
    main.cpp
    database.cpp
    sqlite3.c
)
target_link_libraries(app PRIVATE unofficial::argon2::libargon2)