# Specify the minimum version.
cmake_minimum_required(VERSION 3.9)

# Specify the project info.
project(sensible-test VERSION 1.0.0 DESCRIPTION "Sensible test library")

# Declare the library target.
add_library(${PROJECT_NAME} SHARED
  src/sensible-test.c
  include/sensible-test.h
)

# Configure the directories to search for header files.
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Set the version property.
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# Set the shared object version property to the project's major version.
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

# Set the public header property to the one with the actual API.
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/sensible-test.h)
