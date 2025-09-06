cmake_minimum_required(VERSION 3.5)

project(sendvexjalil)

find_package(vexanium.cdt)

add_contract( sendvexjalil sendvexjalil 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sendvexjalil.cpp
)

target_include_directories( sendvexjalil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
