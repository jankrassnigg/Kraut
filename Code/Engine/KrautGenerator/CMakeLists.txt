#cmake

#
# Copy this file into the folder of an engine library to add it to the solution.
# Below there are configuration options to add preprocessor definitions and link dependencies to the library.
#
# *** Modify this file as required ***
#

if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

SET (LINK_LIBRARIES 
  KrautFoundation
)

include (${CMAKE_SOURCE_DIR}/Code/BuildSystem/CMake/CMAKE_Library.txt)