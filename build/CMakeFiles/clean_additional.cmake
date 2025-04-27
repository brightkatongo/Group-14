# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\FCFSScheduler_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\FCFSScheduler_autogen.dir\\ParseCache.txt"
  "FCFSScheduler_autogen"
  )
endif()
