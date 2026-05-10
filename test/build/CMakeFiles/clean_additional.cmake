# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles/GenealogySystem_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/GenealogySystem_autogen.dir/ParseCache.txt"
  "CMakeFiles/Stage2Smoke_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Stage2Smoke_autogen.dir/ParseCache.txt"
  "GenealogySystem_autogen"
  "Stage2Smoke_autogen"
  )
endif()
