#cmake

#
# Copy this file into the folder of an engine library to add it to the solution.
# Below there are configuration options to add preprocessor definitions and link dependencies to the library.
#
# *** Modify this file as required ***
#

# Add all the preprocessor definitions that are needed to configure this library
# add_definitions (-DMY_PREPROCESS_DEFINITION_1)
# add_definitions (-DMY_PREPROCESS_DEFINITION_2)

# Add all the libraries that this project needs to link against here
# SET (LINK_LIBRARIES
#  Foundation      # Example: Another library build with cmake
#  OpenGL32.lib    # Example: A library that can be found in any lib path, but was not built with cmake
# )

if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

SET (LINK_LIBRARIES 
  KrautFoundation
  KrautGraphics
)

QT_MODULES(Core Gui Widgets OpenGL)

# Call this in case this library does not function without Qt.
BUILD_REQUIRES_QT()


include (${CMAKE_SOURCE_DIR}/Code/BuildSystem/CMake/CMAKE_Library.txt)