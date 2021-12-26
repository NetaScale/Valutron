project(SCAL/UX)
cmake_minimum_required(VERSION 3.9)

#
# functions
#

function(FlexComp path)
GET_FILENAME_COMPONENT(file ${path} NAME_WE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.ll.hh
          ${CMAKE_CURRENT_BINARY_DIR}/${file}.ll.cc
      COMMAND ${FLEX_EXECUTABLE}
      ARGS -o${CMAKE_CURRENT_BINARY_DIR}/${file}.ll.cc
          --header-file=${CMAKE_CURRENT_BINARY_DIR}/${file}.ll.hh
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

#
# feature checks
#

find_package(FLEX)
find_package(BISON)

add_subdirectory(lib/mps)
add_subdirectory(lib/vm)
add_subdirectory(cmd/simulate)

