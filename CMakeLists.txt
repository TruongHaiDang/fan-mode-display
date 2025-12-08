cmake_minimum_required(VERSION 3.16)
project(fan-mode-display LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(fan-mode-display
    src/main.cpp
    ui/mainpopup.cpp
)

target_include_directories(fan-mode-display
    PRIVATE
        inc
        ui
        core
)

target_link_libraries(fan-mode-display PRIVATE Qt6::Widgets)
