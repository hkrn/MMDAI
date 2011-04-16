cmake_minimum_required(VERSION 2.6)

# set library version
set(HTS_ENGINE_API_VERSION 1.02)

project(HTSEngine)
aux_source_directory(lib libHTSEngine_source)
include_directories(include)
add_library(HTSEngine ${libHTSEngine_source})
set_target_properties(HTSEngine PROPERTIES VERSION ${HTS_ENGINE_API_VERSION})
set_target_properties(HTSEngine PROPERTIES SO_VERSION ${HTS_ENGINE_API_VERSION})

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
else()
  set(LIB_TYPE STATIC)
endif()

option(EMBEDDED "Turn on compiling for embedded devices" OFF)
if(EMBEDDED)
  add_definitions(-DHTS_EMBEDDED)
endif()

option(FESTIVAL "Use memory allocation/free functions of speech tools" OFF)
if(FESTIVAL)
  add_definitions(-DFESTIVAL)
endif()

set(libHTSEngine_public_headers include/HTSEngine.h)

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS HTSEngine DESTINATION .)
    set_target_properties(HTSEngine PROPERTIES FRAMEWORK true)
    set_target_properties(HTSEngine PROPERTIES PUBLIC_HEADER "${libHTSEngine_public_headers}")
  endif()
  set_target_properties(HTSEngine PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()


if(NOT MSVC)
  install(TARGETS HTSEngine DESTINATION lib)
  install(DIRECTORY include/ DESTINATION include PATTERN "*.h" PATTERN ".svn" EXCLUDE)
endif()

