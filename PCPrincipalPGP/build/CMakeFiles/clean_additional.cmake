# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\PCPrincipalPGP_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\PCPrincipalPGP_autogen.dir\\ParseCache.txt"
  "PCPrincipalPGP_autogen"
  )
endif()
