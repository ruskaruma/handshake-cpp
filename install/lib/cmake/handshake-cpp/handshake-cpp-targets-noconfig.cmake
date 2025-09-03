#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "handshake-cpp::handshake-cpp" for configuration ""
set_property(TARGET handshake-cpp::handshake-cpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(handshake-cpp::handshake-cpp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libhandshake-cpp.a"
  )

list(APPEND _cmake_import_check_targets handshake-cpp::handshake-cpp )
list(APPEND _cmake_import_check_files_for_handshake-cpp::handshake-cpp "${_IMPORT_PREFIX}/lib/libhandshake-cpp.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
