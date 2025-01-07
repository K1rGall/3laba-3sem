cmake_minimum_required(VERSION 3.28.1)
project(3_laba_3_sem)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.8.0/mingw_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Charts
        REQUIRED)
add_executable(3_laba_3_sem main.cpp
        test_btree.cpp
        test.cpp
        menu.cpp
        test_hashtable.h)
target_link_libraries(3_laba_3_sem
        Qt::Core
        Qt::Gui
        Qt6::Widgets
        Qt6::Charts
)