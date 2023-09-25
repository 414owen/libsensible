add_executable(${PROJECT_NAME}-suite main.c)
target_link_libraries(${PROJECT_NAME}-suite ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-suite PRIVATE ../include)

add_custom_target(check
  COMMAND ${PROJECT_NAME}-suite
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Run test suite in ${CMAKE_CURRENT_SOURCE_DIR}"
)
