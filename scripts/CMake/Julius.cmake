#
# CMake configuration for julius
#
# FIXME: configuration of NetAudio support for libsent
#
cmake_minimum_required(VERSION 2.6)

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)

# set library version
set(VERSION 4.1.5)

# build configuration
project(julius)
aux_source_directory(libjulius/src libjulius_source)
aux_source_directory(libsent/src/anlz libsent_anlz_source)
aux_source_directory(libsent/src/dfa libsent_dfa_source)
aux_source_directory(libsent/src/hmminfo libsent_hmminfo_source)
aux_source_directory(libsent/src/net libsent_net_source)
aux_source_directory(libsent/src/ngram libsent_ngram_source)
aux_source_directory(libsent/src/phmm libsent_phmm_source)
aux_source_directory(libsent/src/util libsent_util_source)
aux_source_directory(libsent/src/voca libsent_voca_source)
aux_source_directory(libsent/src/wav2mfcc libsent_wav2mfcc_source)
include_directories(libjulius/include libsent/include)

# create config.h.cmake
file(READ libjulius/include/julius/config.h.in JULIUS_CONFIG_H_IN)
file(READ libsent/include/sent/config.h.in SENT_CONFIG_H_IN)

set(JULIUS_PRODUCTNAME "\"juliusLib\"")
set(JULIUS_VERSION "\"${VERSION}\"")
set(JULIUS_HOSTINFO "\"\"")
string(REPLACE "#undef JULIUS_PRODUCTNAME" "#define JULIUS_PRODUCTNAME @JULIUS_PRODUCTNAME@" JULIUS_CONFIG_H_IN ${JULIUS_CONFIG_H_IN})
string(REPLACE "#undef JULIUS_VERSION" "#define JULIUS_VERSION @JULIUS_VERSION@" JULIUS_CONFIG_H_IN ${JULIUS_CONFIG_H_IN})
string(REPLACE "#undef JULIUS_HOSTINFO" "#define JULIUS_HOSTINFO @JULIUS_HOSTINFO@" JULIUS_CONFIG_H_IN ${JULIUS_CONFIG_H_IN})
string(REPLACE "#undef JULIUS_SETUP" "#define JULIUS_SETUP @JULIUS_SETUP@" JULIUS_CONFIG_H_IN ${JULIUS_CONFIG_H_IN})

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
else()
  set(LIB_TYPE STATIC)
endif()

# libjulius configuration
option(UNIGRAM_FACOTRING "use 1-gram factoring on 1st pass" OFF)
option(BIGRAM_FACOTRING "use 2-gram factoring on 1st pass" OFF)
option(PASS1_IWCD "handle inter-word triphone on 1st pass" OFF)
option(PASS2_STRICT_ICWD "strict IWCD scoring on 2nd pass" OFF)
if(BIGRAM_FACOTRING)
  set(UNIGRAM_FACOTRING OFF BOOL)
endif()

option(ALGORITHM_SET_STANDARD "high accuracy, slow speed" OFF)
option(ALGORITHM_SET_FAST "balanced for both speed and accuracy" ON)
if(ALGORITHM_SET_FAST)
  set(UNIGRAM_FACOTRING ON BOOL)
  set(PASS1_IWCD ON BOOL)
  set(PASS2_STRICT_ICWD ON BOOL)
  set(GPRUNE_DEFAULT "safe" INTERNAL)
  set(JULIUS_SETUP "\"fast\"")
elseif(ALGORITHM_SET_STANDARD)
  set(UNIGRAM_FACOTRING ON BOOL)
  set(PASS1_IWCD ON BOOL)
  set(PASS2_STRICT_ICWD ON BOOL)
  set(GPRUNE_DEFAULT "\"beam\"" INTERNAL)
  set(JULIUS_SETUP "\"standard\"")
endif()
if(GPRUNE_DEFAULT STREQUAL "safe")
  set(GPRUNE_DEFAULT_SAFE ON BOOL)
elseif(GPRUNE_DEFAULT STREQUAL "beam")
  set(GPRUNE_DEFAULT_BEAM ON BOOL)
endif()

option(LOWMEM "all words share a single root on lexicon tree" OFF)
option(LOWMEM2 "separate hi-freq words from lexicon tree" OFF)
option(MONOTREE "monophone lexicon on 1st pass (EXPERIMENTAL)" OFF)
option(SCORE_BEAM "enable score envelope beaming on 2nd pass scan" ON)
option(WPAIR "use word-pair approximation on 1st pass" OFF)
option(WPAIR_NLIMIT "keep only N-best path with wpair (-nlimit)" OFF)
if(SCORE_BEAM)
  set(SCAN_BEAM ON BOOL)
endif()
if(WPAIR_NLIMIT)
  set(WPAIR ON BOOL)
  set(WPAIR_KEEP_NLIMIT ON BOOL)
endif()

option(WORD_GRAPH "use word graph instead of trellis between passes" OFF)
option(GMM_VAD "GMM-based VAD (EXPERIMENTAL)" OFF)
if(GMM_VAD)
  set(GMM_VAD ON BOOL)
  set(BACKEND_VAD ON BOOL)
endif()

option(DECODER_VAD "a new decoder-based VAD by NAIST team" OFF)
if(DECODER_VAD)
  set(SPSEGMENT_NAIST ON BOOL)
  set(BACKEND_VAD ON BOOL)
endif()

option(POWER_REJECT "post rejection by power" OFF)

option(GRAPHOUT_NBEST "word graph output from N-best sentence" OFF)
if(NOT GRAPHOUT_NBEST)
  set(GRAPHOUT_DYNAMIC ON BOOL)
  set(GRAPHOUT_SEARCH ON BOOL)
endif()

option(LMFIX "" ON)
if(LMFIX)
  set(LM_FIX_DOUBLE_SCORING ON BOOL)
endif()

option(ENABLE_PLUGIN "plugin support" ON)
if(ENABLE_PLUGIN)
  check_library_exists(dl dlopen "" HAVE_DLOPEN)
  if(HAVE_DLOPEN)
    set(ENABLE_PLUGIN ON BOOL)
  endif()
endif()

option(ENABLE_THREAD "use thread" ON)
if(ENABLE_THREAD AND NOT MINGW)
  include(FindThreads)
  set(HAVE_PTHREAD CMAKE_USE_PTHREADS_INIT)
endif()

option(CONFIDENCE_MEASURE "confidence measure computation" ON)
option(CM_NBEST "N-best CM instead of search CM" OFF)
option(CM_MULTIPLE_ALPHA "EXPERIMENTAL: test multi alphas (need much mem)" OFF)
option(CMTHRES "confidence score based pruning on 2nd pass" OFF)
if(CM_NBEST)
  if(CONFIDENCE_MEASURE)
    set(CM_BEST BOOL ON)
  else()
    message(FATAL_ERROR "CM AND JULIUS_CM_NBEST conflicts")
  endif()
endif()
if(CM_MULTIPLE_ALPHA)
  if(CONFIDENCE_MEASURE)
    set(CM_MULTIPLE_ALPHA BOOL ON)
  else()
    message(FATAL_ERROR "CM AND JULIUS_CM_MULTIPLE_ALPHA conflicts")
  endif()
endif()
if(CMTHRES)
  if(CONFIDENCE_MEASURE)
    if(CM_NBEST)
      message(FATAL_ERROR "CMTHRES cannot be used with JULIUS_CM_NBEST")
    elseif(CM_MULTIPLE_ALPHA)
      message(FATAL_ERROR "CMTHRES cannot be used with JULIUS_CM_MULTIPLE_ALPHA")
    else()
      set(CM_SEARCH_LIMIT ON)
    endif()
  else()
    message(FATAL_ERROR "CM and JULIUS_CMTHRES conflicts")
  endif()
endif()


# libsent configuration
set(libsent_adin_source "libsent/src/adin/adin_file.c"
                        "libsent/src/adin/adin_tcpip.c"
                        "libsent/src/adin/ds48to16.c"
                        "libsent/src/adin/zc-e.c"
                        "libsent/src/adin/zmean.c")

option(USE_ADD_ARRAY "addlog_array() function" ON)
if(USE_ADD_ARRAY)
  set(USE_ADD_ARRAY ON BOOL)
endif()

option(WORDS_INT "integer instead of unsigned short for word ID to extend vocabulary limit to 2^31=2G words" OFF)
if(WORDS_INT)
  set(WORDS_INT ON BOOL)
endif()

option(CLASS_NGRAM "class N-gram support" ON)
if(CLASS_NGRAM)
  set(CLASS_NGRAM ON BOOL)
endif()

option(FORK_ADINNET "process forking on adinnet" OFF)
if(FORK_ADINNET)
  set(FORK_ADINNET ON BOOL)
endif()

option(MFCC_SINCOS_TABLE "sin/cos table for MFCC calculation" ON)
if(MFCC_SINCOS_TABLE)
  set(MFCC_SINCOS_TABLE ON BOOL)
endif()

option(ENABLE_MSD "MSD model support" OFF)
if(ENABLE_MSD)
  set(ENABLE_MSD ON BOOL)
endif()

option(LINK_ZLIB "link against with zlib" ON)
if(LINK_ZLIB)
  find_package(ZLIB)
  if(ZLIB_FOUND)
    list(APPEND LIBSENT_DEPENDS_INCLUDE ${ZLIB_INCLUDE_DIR})
    list(APPEND LIBSENT_DEPENDS_LIBRARY ${ZLIB_LIBRARIES})
    set(GZDESC "zlib library")
  endif()
endif()

find_program(GZIP_BIN "gzip")
if(GZIP_BIN)
  set(ZCAT "\"gzip -d -c\"")
else()
  set(ZCAT "\"\"")
endif()
if(NOT ZLIB_FOUND)
  set(GZDESC "gzip command")
elseif(NOT ZLIB_FOUND AND NOT GZIP_BIN)
  set(GZDESC "none")
endif()

set(WAVEFILE_SUPPORT "RAW and WAV only")
option(LINK_SNDFILE "link against with libsndfile" ON)
if(LINK_SNDFILE)
  find_path(LIBSNDFILE_INCLUDE sndfile.h)
  find_library(LIBSNDFILE_LIBRARY sndfile)
  if(LIBSNDFILE_INCLUDE AND LIBSNDFILE_LIBRARY)
    list(APPEND LIBSENT_DEPENDS_INCLUDE ${LIBSNDFILE_INCLUDE})
    list(APPEND LIBSENT_DEPENDS_LIBRARY ${LIBSNDFILE_LIBRARY})
    list(APPEND libsent_adin_source "libsent/src/adin/adin_sndfile.c")
    set(WAVEFILE_SUPPORT "various formats by libsndfile ver.1")
    set(HAVE_LIBSNDFILE ON BOOL)
    set(HAVE_LIBSNDFILE_VER1 ON BOOL)
  endif()
endif()

set(ALTYPE "portaudio")
if(ALTYPE STREQUAL "portaudio" OR ALTYPE MATCHES "pa-*")
  include(CheckLibraryExists)
  find_path(PA_INCLUDE portaudio.h)
  find_library(PA_LIBRARY portaudio)
  if(PA_INCLUDE AND PA_LIBRARY)
    list(APPEND libsent_adin_source "libsent/src/adin/adin_portaudio.c")
    list(APPEND LIBSENT_DEPENDS_INCLUDE ${PA_INCLUDE})
    list(APPEND LIBSENT_DEPENDS_LIBRARY ${PA_LIBRARY})
    set(ALDESC "PortAudio library (external)")
  else()
    list(APPEND libsent_adin_source "libsent/src/adin/adin_portaudio.c"
                                    "libsent/src/adin/pa/pa_lib.c"
                                    "libsent/src/adin/pa/pa_convert.c"
                                    "libsent/src/adin/pa/pa_trace.c"
                                    "libsent/src/adin/pa/pablio.c"
                                    "libsent/src/adin/pa/ringbuffer.c")
    include_directories("libsent/src/adin/pa")
    if(ALTYPE STREQUAL "pa-winmm")
      find_library(WINMM_LIBRARY "winmm")
      list(APPEND libsent_adin_source "libsent/src/adin/pa/pa_win_wmme.c")
      list(APPEND LIBSENT_DEPENDS_LIBRARY ${WINMM_LIBRARY})
    elseif(ALTYPE STREQUAL "pa-dsound")
      find_library(WINMM_LIBRARY "winmm")
      find_library(DSOUND_LIBRARY "dsound")
      list(APPEND libsent_adin_source "libsent/src/adin/pa/dsound_wrapper.c")
      list(APPEND LIBSENT_DEPENDS_LIBRARY ${WINMM_LIBRARY} ${DSOUND_LIBRARY})
    elseif(ALTYPE STREQUAL "pa-oss")
      list(APPEND libsent_adin_source "libsent/src/adin/pa/pa_unix.c"
                                      "libsent/src/adin/pa/pa_unix_oss.c")
    elseif(ALTYPE STREQUAL "pa-solaris")
      list(APPEND libsent_adin_source "libsent/src/adin/pa/pa_unix_solaris.c")
    endif()
  endif()
  set(USE_MIC ON BOOL)
endif()

if(APPLE)
  find_library(AUDIO_UNIT_FRAMEWORK "AudioUnit")
  find_library(AUDIO_TOOLBOX_FRAMEWORK "AudioToolbox")
  find_library(CORE_AUDIO_FRAMEWORK "CoreAudio")
  find_library(CORE_SERVICES_FRAMEWORK "CoreServices")
  list(APPEND LIBSENT_DEPENDS_LIBRARY ${AUDIO_UNIT_FRAMEWORK}
                                      ${AUDIO_TOOLBOX_FRAMEWORK}
                                      ${CORE_AUDIO_FRAMEWORK}
                                      ${CORE_SERVICES_FRAMEWORK})
endif()

check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strcasecmp HAVE_SLEEP)
find_library(SOCKET_LIBRARY socket)
if(SOCKET_LIBRARY)
  set(HAVE_LIBSOCKET ON BOOL)
  list(APPEND LIBSENT_DEPENDS_LIBRARY ${SOCKET_LIBRARY})
endif()
find_library(ICONV_LIBRARY iconv)
if(ICONV_LIBRARY)
  set(HAVE_ICONV ON BOOL)
  list(APPEND LIBSENT_DEPENDS_LIBRARY ${ICONV_LIBRARY})
endif()

set(LIBSENT_VERSION "\"${VERSION}\"")
set(AUDIO_API_NAME "\"${ALTYPE}\"")
set(AUDIO_API_DESC "\"${ALDESC}\"")
set(AUDIO_FORMAT_DESC "\"${WAVEFILE_SUPPORT}\"")
set(GZIP_READING_DESC "\"${GZDESC}\"")
string(REPLACE "#undef LIBSENT_VERSION" "#define LIBSENT_VERSION @LIBSENT_VERSION@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})
string(REPLACE "#undef AUDIO_API_NAME" "#define AUDIO_API_NAME @AUDIO_API_NAME@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})
string(REPLACE "#undef AUDIO_API_DESC" "#define AUDIO_API_DESC @AUDIO_API_DESC@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})
string(REPLACE "#undef AUDIO_FORMAT_DESC" "#define AUDIO_FORMAT_DESC @AUDIO_FORMAT_DESC@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})
string(REPLACE "#undef GZIP_READING_DESC" "#define GZIP_READING_DESC @GZIP_READING_DESC@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})
string(REPLACE "#undef ZCAT" "#define ZCAT @ZCAT@" SENT_CONFIG_H_IN ${SENT_CONFIG_H_IN})

add_library(julius ${libjulius_source} ${libsent_anlz_source}
                   ${libsent_dfa_source} ${libsent_hmminfo_source}
                   ${libsent_net_source} ${libsent_ngram_source}
                   ${libsent_phmm_source} ${libsent_util_source}
                   ${libsent_voca_source} ${libsent_wav2mfcc_source}
                   ${libsent_adin_source})

target_link_libraries(julius ${LIBSENT_DEPENDS_LIBRARY})
include_directories(julius ${LIBSENT_DEPENDS_INCLUDE})

set_target_properties(julius PROPERTIES VERSION ${VERSION})
set_target_properties(julius PROPERTIES SO_VERSION ${VERSION})

# rewrite configuration
set(CC ${CMAKE_C_COMPILER})
set(CFLAGS ${CMAKE_C_FLAGS})
string(REPLACE "#undef" "#cmakedefine" JULIUS_CONFIG_H_CMAKE ${JULIUS_CONFIG_H_IN})
string(REPLACE "#undef" "#cmakedefine" SENT_CONFIG_H_CMAKE ${SENT_CONFIG_H_IN})
file(WRITE libjulius/include/julius/config.h.cmake ${JULIUS_CONFIG_H_CMAKE})
file(WRITE libsent/include/sent/config.h.cmake ${SENT_CONFIG_H_CMAKE})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libjulius/include/julius/config.h.cmake 
               ${CMAKE_CURRENT_SOURCE_DIR}/libjulius/include/julius/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libsent/include/sent/config.h.cmake 
               ${CMAKE_CURRENT_SOURCE_DIR}/libsent/include/sent/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libjulius/src/version.c.in
               ${CMAKE_CURRENT_SOURCE_DIR}/libjulius/src/version.c)
file(REMOVE libjulis/include/julius/config.h.cmake libsent/include/sent/config.h.cmake)

file(GLOB libjulius_headers libjulius/include/julius/*.h)
file(GLOB libsent_headers libsent/include/sent/*.h)
set(libjulius_public_headers ${libjulius_headers} ${libsent_headers})


# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS julius DESTINATION .)
    set_target_properties(julius PROPERTIES FRAMEWORK true)
    set_target_properties(julius PROPERTIES PUBLIC_HEADER "${libjulius_public_headers}")
  endif()
  set_target_properties(julius PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(MINGW)
  find_library(WINSOCK2_LIBRARY ws2_32)
  target_link_libraries(julius ${WINSOCK2_LIBRARY})
endif()

if(NOT MSVC)
  install(TARGETS julius DESTINATION lib)
  install(DIRECTORY libjulius/include/ libsent/include/
          DESTINATION include
          PATTERN "*.h"
          PATTERN "*.h.in" EXCLUDE
          PATTERN "*.cmake" EXCLUDE
          PATTERN ".svn" EXCLUDE)
endif()

