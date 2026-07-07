cmake_minimum_required(VERSION 3.16)
project(ExpenseTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    main.cpp
    mainwindow.cpp
    loginwidget.cpp
    signupwidget.cpp
    dashboard.cpp
    database.cpp
)

set(HEADERS
    mainwindow.h
    loginwidget.h
    signupwidget.h
    dashboard.h
    database.h
    widgets.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})