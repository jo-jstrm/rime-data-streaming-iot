# Try to find CAF headers and libraries.
#
# Use this module as follows:
#
#     find_package(CAF [COMPONENTS <core|io|opencl|...>*] [REQUIRED])
#
# Variables used by this module (they can change the default behaviour and need
# to be set before calling find_package):
#
#  CAF_ROOT_DIR  Set this variable either to an installation prefix or to wa
#                CAF build directory where to look for the CAF libraries.
#
# Variables defined by this module:
#
#  CAF_FOUND              System has CAF headers and library
#  CAF_VERSION            Found CAF release number
#  CAF_LIBRARIES          List of library files  for all components
#  CAF_INCLUDE_DIRS       List of include paths for all components

if(CAF_FIND_COMPONENTS STREQUAL "")
  message(FATAL_ERROR "FindCAF requires at least one COMPONENT.")
endif()

# iterate over user-defined components
foreach (comp ${CAF_FIND_COMPONENTS})
  # we use uppercase letters only for variable names
  string(TOUPPER "${comp}" UPPERCOMP)
  if ("${comp}" STREQUAL "core")
    set(HDRNAME "caf/all.hpp")
  elseif ("${comp}" STREQUAL "test")
    set(HDRNAME "caf/test/unit_test.hpp")
  else ()
    set(HDRNAME "caf/${comp}/all.hpp")
  endif ()
  if (CAF_ROOT_DIR)
    set(header_hints
        "${CAF_ROOT_DIR}/include"
        "${CAF_ROOT_DIR}/libcaf_${comp}"
        "${CAF_ROOT_DIR}/../libcaf_${comp}"
        "${CAF_ROOT_DIR}/../../libcaf_${comp}")
  endif ()
  find_path(CAF_INCLUDE_DIR_${UPPERCOMP}
            NAMES
              ${HDRNAME}
            HINTS
              ${header_hints}
              /usr/include
              /usr/local/include
              /opt/local/include
              /sw/include
              ${CMAKE_INSTALL_PREFIX}/include
              ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
  mark_as_advanced(CAF_INCLUDE_DIR_${UPPERCOMP})
  if (NOT "${CAF_INCLUDE_DIR_${UPPERCOMP}}"
      STREQUAL "CAF_INCLUDE_DIR_${UPPERCOMP}-NOTFOUND")
    # mark as found (set back to false when missing library or build header)
    set(CAF_${comp}_FOUND true)
    # check for CMake-generated build header for the core component
    if ("${comp}" STREQUAL "core")
      find_path(caf_build_header_path
                NAMES
                  caf/detail/build_config.hpp
                HINTS
                  ${header_hints}
                  /usr/include
                  /usr/local/include
                  /opt/local/include
                  /sw/include
                  ${CMAKE_INSTALL_PREFIX}/include
                  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
      if ("${caf_build_header_path}" STREQUAL "caf_build_header_path-NOTFOUND")
        message(WARNING "Found all.hpp for CAF core, but not build_config.hpp")
        set(CAF_${comp}_FOUND false)
      else()
        list(APPEND CAF_INCLUDE_DIRS "${caf_build_header_path}")
      endif()
    endif()
    list(APPEND CAF_INCLUDE_DIRS "${CAF_INCLUDE_DIR_${UPPERCOMP}}")
    # look for (.dll|.so|.dylib) file, again giving hints for non-installed CAFs
    # skip probe_event as it is header only
    if (NOT ${comp} STREQUAL "probe_event" AND NOT ${comp} STREQUAL "test")
      if (CAF_ROOT_DIR)
        set(library_hints "${CAF_ROOT_DIR}/lib")
      endif ()
      find_library(CAF_LIBRARY_${UPPERCOMP}
                   NAMES
                     "caf_${comp}"
                     "caf_${comp}_static"
                   HINTS
                     ${library_hints}
                     /usr/lib
                     /usr/local/lib
                     /opt/local/lib
                     /sw/lib
                     ${CMAKE_INSTALL_PREFIX}/lib
                     ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
                     ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_BUILD_TYPE})
      mark_as_advanced(CAF_LIBRARY_${UPPERCOMP})
      if ("${CAF_LIBRARY_${UPPERCOMP}}"
          STREQUAL "CAF_LIBRARY_${UPPERCOMP}-NOTFOUND")
        set(CAF_${comp}_FOUND false)
      else ()
        set(CAF_LIBRARIES ${CAF_LIBRARIES} ${CAF_LIBRARY_${UPPERCOMP}})
      endif ()
    endif ()
  endif ()
endforeach ()

if (DEFINED CAF_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES CAF_INCLUDE_DIRS)
endif()


if (NOT CAF_INCLUDE_DIR_CORE STREQUAL "CAF_INCLUDE_DIR_CORE-NOTFOUND")
  # read content of config.hpp
  file(READ "${CAF_INCLUDE_DIR_CORE}/caf/config.hpp" CONFIG_HPP)
  # get line containing the version
  string(REGEX MATCH "#define CAF_VERSION [0-9]+" VERSION_LINE "${CONFIG_HPP}")
  # extract version number from line
  string(REGEX MATCH "[0-9]+" VERSION_INT "${VERSION_LINE}")
  # calculate major, minor, and patch version
  math(EXPR CAF_VERSION_MAJOR "${VERSION_INT} / 10000")
  math(EXPR CAF_VERSION_MINOR "( ${VERSION_INT} / 100) % 100")
  math(EXPR CAF_VERSION_PATCH "${VERSION_INT} % 100")
  # create full version string
  set(CAF_VERSION "${CAF_VERSION_MAJOR}.${CAF_VERSION_MINOR}.${CAF_VERSION_PATCH}")
  if (NOT CAF_VERSION)
    unset(CAF_VERSION)
    message(WARNING "Unable to determine CAF version")
  endif ()
endif ()

# let CMake check whether all requested components have been found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CAF
                                  FOUND_VAR CAF_FOUND
                                  REQUIRED_VARS CAF_VERSION CAF_LIBRARIES CAF_INCLUDE_DIRS
                                  HANDLE_COMPONENTS)

# final step to tell CMake we're done
mark_as_advanced(CAF_ROOT_DIR
                 CAF_VERSION
                 CAF_LIBRARIES
                 CAF_INCLUDE_DIRS)