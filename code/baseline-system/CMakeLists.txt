cmake_minimum_required(VERSION 3.16)

project(rime-baseline VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --------------------------------------------------------------------------------
#                         Locate files
# --------------------------------------------------------------------------------

set(SOURCES  
  src/custom_types/attribute.cpp
  src/custom_types/net_init.cpp
  src/custom_types/node_address.cpp
  src/custom_types/node_info.cpp
  src/custom_types/query.cpp
  src/custom_types/results.cpp
  src/log/logger.cpp
  src/networking/network_manager.cpp
  src/sensors/sensor_location.cpp
  src/sensors/sensor_random.cpp
  src/sensors/sensor_state.cpp
  src/sensors/sensor_state_location.cpp
  src/utility/find_node_info.cpp   
  src/base_station.cpp
  src/srt_config.cpp
)

set(LIBRARY_NAME librime)

# Add compiler flag "-pthread"
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Defines the path where CMake will search for modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Will search for FindCAF.cmake and FindThreads in ${CMAKE_MODULE_PATH}
find_package(Threads REQUIRED)
find_package(CAF 0.17.6 EXACT COMPONENTS core io test REQUIRED)

if (Threads_FOUND)
  message(STATUS "Found pthreads")
else()
  message(FATAL_ERROR "pthreads not found")
endif()
if (CAF_FOUND)
  message(STATUS "Found CAF")
  message(STATUS "CAF Libraries: " ${CAF_LIBRARIES})
  message(STATUS "CAF Include Paths: " ${CAF_INCLUDE_DIRS} \n)
else ()
  message(FATAL_ERROR "CAF not found")
endif ()

# --------------------------------------------------------------------------------
#                                   Build
# --------------------------------------------------------------------------------

# ----------------------------------lib-------------------------------------------
# Compile all sources into a library.
add_library(${LIBRARY_NAME} OBJECT ${SOURCES})
# Tells compiler to look for headers in "/include/*"
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/custom_types
  ${PROJECT_SOURCE_DIR}/include/log
  ${PROJECT_SOURCE_DIR}/include/networking
  ${PROJECT_SOURCE_DIR}/include/query_processing
  ${PROJECT_SOURCE_DIR}/include/sensors
  ${PROJECT_SOURCE_DIR}/include/utility)

# ----------------------------------app-------------------------------------------
add_executable(${CMAKE_PROJECT_NAME} app/main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LIBRARY_NAME} ${CAF_LIBRARIES} Threads::Threads)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
