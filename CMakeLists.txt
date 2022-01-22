project(Valutron)
cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

#
# functions
#

function(FlexComp path)
    GET_FILENAME_COMPONENT(file ${path} NAME_WE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.l.hh
          ${CMAKE_CURRENT_BINARY_DIR}/${file}.l.cc
      COMMAND ${FLEX_EXECUTABLE}
      ARGS -o${CMAKE_CURRENT_BINARY_DIR}/${file}.l.cc
          --header-file=${CMAKE_CURRENT_BINARY_DIR}/${file}.l.hh
          ${CMAKE_CURRENT_SOURCE_DIR}/${path}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${path})
endfunction(FlexComp)

function(BisonComp path)
    GET_FILENAME_COMPONENT(file ${path} NAME_WE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.cc
            ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.hh
        ${CMAKE_CURRENT_BINARY_DIR}/${file}.output
        COMMAND ${BISON_EXECUTABLE}
        ARGS --verbose -Wall
            -o ${CMAKE_CURRENT_BINARY_DIR}/${file}.tab.cc -d ${path}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${path}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction(BisonComp)

function (LemonComp path)
    GET_FILENAME_COMPONENT (file ${path} NAME_WE)
    add_custom_command(
        OUTPUT ${file}.tab.cc ${file}.tab.h ${file}.out
        COMMAND $<TARGET_FILE:lemonxx>
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${path}
            -d. -p -T${CMAKE_SOURCE_DIR}/tools/lemon/lemxx.tpl
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${path})
endfunction (LemonComp)

#
# feature checks
#

find_package(Threads REQUIRED)
find_package(FLEX)
#find_package(BISON)


if (NOT FLEX_FOUND)
        set(FLEX_FOUND true)
        set(FLEX_EXECUTABLE ${PROJECT_SOURCE_DIR}/tools/win_flex/win_flex.exe)
        message(STATUS "Flex for Windows used: " ${FLEX_EXECUTABLE})
endif ()

add_subdirectory(tools/lemon)
add_subdirectory(lib/mps)
add_subdirectory(cmd/vm)

