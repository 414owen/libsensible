add_executable(${PROJECT_NAME}-suite
  main.c
)
target_link_libraries(${PROJECT_NAME}-suite ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-suite PRIVATE ../include)
