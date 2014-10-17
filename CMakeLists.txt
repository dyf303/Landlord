# Set projectname (must be done AFTER setting configurationtypes)
project(Landlord)

cmake_minimum_required(VERSION 2.8.9)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

# set default buildoptions and print them
include(cmake/options.cmake)

include(CheckPlatform)

find_package(PCHSupport)

include(ConfigureBoost)

add_subdirectory(src)

 

