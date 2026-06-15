cmake_minimum_required(VERSION 3.16)
project(ExpenseTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add Sql here
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

# Add database files here
add_executable(ExpenseTracker 
    main.cpp
    mainwindow.cpp
    loginwidget.cpp
    signupwidget.cpp
    database.cpp
)

add_custom_command(TARGET ExpenseTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/bg_left.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/bg_left.jpg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/bg_right.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/bg_right.jpg
)

# Add Qt6::Sql here
target_link_libraries(ExpenseTracker PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)