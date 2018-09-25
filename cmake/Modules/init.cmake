if (NOT CMAKE_RULE_MESSAGES)
  message(STATUS "Setting default CMAKE_RULE_MESSAGES to OFF")
  set(CMAKE_RULE_MESSAGES OFF)
endif ()

if (NOT CMAKE_VERBOSE_MAKEFILE)
  message(STATUS "Setting default CMAKE_VERBOSE_MAKEFILE to ON")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting default CMAKE_BUILD_TYPE to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 14)

####################################################################################################
# Cross-compilation {

set(BOARD_FAMILY $ENV{BOARD_FAMILY})

if (NOT BOARD_FAMILY)
  message(STATUS "Setting default BOARD_FAMILY to x86 (options: stm32 | arduino | x86")
  set(BOARD_FAMILY "x86")
endif()

add_definitions(-D${BOARD_FAMILY})

# }

####################################################################################################
# Unit testing {

if (NOT ENABLE_TESTS)
  set(ENABLE_TESTS ON)
endif()

if (ENABLE_TESTS)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()

# }
