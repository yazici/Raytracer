MESSAGE("Generating Makefile for Core project")

FILE(GLOB_RECURSE RT_CORE_SOURCES *.cpp)
FILE(GLOB_RECURSE RT_CORE_HEADERS *.hpp)
FILE(GLOB_RECURSE RT_EXTERNAL_TINYEXR_SOURCES ../External/tinyexr/*.cc)

# setup directories
INCLUDE_DIRECTORIES(${RT_CORE_DIRECTORY})

ADD_LIBRARY(Core SHARED ${RT_CORE_SOURCES} ${RT_CORE_HEADERS} ${RT_EXTERNAL_TINYEXR_SOURCES})

ADD_CUSTOM_COMMAND(TARGET Core POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Core> ${RT_OUTPUT_DIRECTORY}/${targetfile})