FlexComp(Scanner.l)
LemonComp(Parser.y)

add_executable(vm  AST.cc Main.cc ObjectMemory.cc Objects.cc
    Synth.cc
    ${CMAKE_CURRENT_BINARY_DIR}/Scanner.l.cc
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cc)

target_include_directories(vm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/tools)
target_link_libraries(vm mps)