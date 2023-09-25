add_executable(${PROJECT_NAME}-suite main.c)
target_link_libraries(${PROJECT_NAME}-suite ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-suite PRIVATE ../include)

add_test(${PROJECT_NAME}-suite-build
  "${CMAKE_COMMAND}"
  --build "${CMAKE_BINARY_DIR}"
  --config "$<CONFIG>"
  --target ${PROJECT_NAME}-suite
)

add_test(NAME ${PROJECT_NAME}-suite COMMAND ${PROJECT_NAME}-suite)

set_tests_properties(${PROJECT_NAME}-suite PROPERTIES DEPENDS ${PROJECT_NAME}-suite-build)
