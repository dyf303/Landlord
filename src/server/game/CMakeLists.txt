
file(GLOB_RECURSE sources_AI AI/*.cpp AI/*.h)
file(GLOB_RECURSE sources_Player Player/*.cpp Player/*.h)
file(GLOB_RECURSE sources_Room Room/*.cpp Room/*.h)
file(GLOB_RECURSE sources_Server Server/*.cpp Server/*.h)
file(GLOB_RECURSE sources_World World/*.cpp World/*.h)

# Create game-libary

if (USE_COREPCH)
  set(game_STAT_PCH_HDR PrecompiledHeaders/gamePCH.h)
  set(game_STAT_PCH_SRC PrecompiledHeaders/gamePCH.cpp)
endif ()

set(game_STAT_SRCS
  ${sources_AI}
  ${sources_Player}
  ${sources_Room}
  ${sources_Server}
  ${sources_World}
)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Networking
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/AI
  ${CMAKE_CURRENT_SOURCE_DIR}/Room
  ${CMAKE_CURRENT_SOURCE_DIR}/Player
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${CMAKE_CURRENT_SOURCE_DIR}/Server/Protocol
  ${CMAKE_CURRENT_SOURCE_DIR}/Server
  ${CMAKE_CURRENT_SOURCE_DIR}/World
)

add_library(game STATIC
  ${game_STAT_SRCS}
  ${game_STAT_PCH_SRC}
)

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(game ${game_STAT_PCH_HDR} ${game_STAT_PCH_SRC})
endif ()

