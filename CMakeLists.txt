# Set projectname (must be done AFTER setting configurationtypes)
project(Landlord)

cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0005 OLD)
# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

# set default buildoptions and print them
include(cmake/options.cmake)

# turn off PCH totally if enabled (hidden setting, mainly for devs)
if( NOPCH )
  set(USE_COREPCH 0)
  set(USE_SCRIPTPCH 0)
endif()

include(CheckPlatform)

find_package(PCHSupport)

include(ConfigureBoost)

add_subdirectory(src)

 

