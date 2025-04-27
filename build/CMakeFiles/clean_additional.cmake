# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\FCFSSimulation_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\FCFSSimulation_autogen.dir\\ParseCache.txt"
  "FCFSSimulation_autogen"
  )
endif()
