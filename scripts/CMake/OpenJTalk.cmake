#
# CMake configuration for OpenJTalk
#
cmake_minimum_required(VERSION 2.6)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

# set library version
set(OPEN_JTALK_VERSION 1.02)

file(WRITE "mecab/src/config.h")

# build configuration
project(OpenJTalk)
aux_source_directory(jpcommon libjpcommon_source)
aux_source_directory(mecab/src libmecab_source)
aux_source_directory(mecab2njd libmecab2njd_source)
aux_source_directory(njd libnjd_source)
aux_source_directory(njd2jpcommon libnjd2jpcommon_source)
aux_source_directory(njd_set_accent_phrase libnjd_set_accent_phrase_source)
aux_source_directory(njd_set_accent_type libnjd_set_accent_type_source)
aux_source_directory(njd_set_digit libnjd_set_digit_source)
aux_source_directory(njd_set_long_vowel libnjd_set_long_vowel_source)
aux_source_directory(njd_set_pronunciation libnjd_set_pronunciation_source)
aux_source_directory(njd_set_unvoiced_vowel libnjd_set_unvoiced_vowel_source)
aux_source_directory(text2mecab libtext2mecab_source)
include_directories(jpcommon mecab/src mecab2njd njd njd2jpcommon njd_set_accent_phrase
                    njd_set_accent_type njd_set_digit njd_set_long_vowel
                    njd_set_pronunciation njd_set_unvoiced_vowel text2mecab)

add_library(OpenJTalk ${libjpcommon_source} ${libmecab_source} ${libmecab2njd_source}
                      ${libnjd_source} ${libnjd2jpcommon_source}
                      ${libnjd_set_accent_phrase_source} ${libnjd_set_accent_type_source}
                      ${libnjd_set_digit_source} ${libnjd_set_long_vowel_source}
                      ${libnjd_set_pronunciation_source} ${libnjd_set_unvoiced_vowel_source}
                      ${libtext2mecab_source})
set_target_properties(OpenJTalk PROPERTIES VERSION ${OPEN_JTALK_VERSION})
set_target_properties(OpenJTalk PROPERTIES SO_VERSION ${OPEN_JTALK_VERSION})

# generate mecab/config.h
check_include_files(ctype.h HAVE_CTYPE_H)
if(HAVE_CTYPE_H)
  add_definitions(-DHAVE_CTYPE_H)
endif()

check_include_files(dirent.h HAVE_DIRENT_H)
if(HAVE_DIRENT_H)
  add_definitions(-DHAVE_DIRENT_H)
endif()

check_include_files(fcntl.h HAVE_FCNTL_H)
if(HAVE_FCNTL_H)
  add_definitions(-DHAVE_FCNTL_H)
endif()

check_include_files(inttypes.h HAVE_INTTYPES_H)
if(HAVE_INTTYPES_H)
  add_definitions(-DHAVE_INTTYPES_H)
endif()

check_include_files(io.h HAVE_IO_H)
if(HAVE_IO_H)
  add_definitions(-DHAVE_IO_H)
endif()

check_include_files(memory.h HAVE_MEMORY_H)
if(HAVE_MEMORY_H)
  add_definitions(-DHAVE_MEMORY_H)
endif()

check_include_files(setjmp.h HAVE_SETJMP_H)
if(HAVE_SETJMP_H)
  add_definitions(-DHAVE_SETJMP_H)
endif()

check_include_files(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
  add_definitions(-DHAVE_STDINT_H)
endif()

check_include_files(stdlib.h HAVE_STDLIB_H)
if(HAVE_STDLIB_H)
  add_definitions(-DHAVE_STDLIB_H)
endif()

check_include_files(strings.h HAVE_STRINGS_H)
if(HAVE_STRINGS_H)
  add_definitions(-DHAVE_STRINGS_H)
endif()

check_include_files(string.h HAVE_STRING_H)
if(HAVE_STRING_H)
  add_definitions(-DHAVE_STRING_H)
endif()

check_include_files(sys/mman.h HAVE_SYS_MMAN_H)
if(HAVE_SYS_MMAN_H)
  add_definitions(-DHAVE_SYS_MMAN_H)
endif()

check_include_files(sys/stat.h HAVE_SYS_STAT_H)
if(HAVE_SYS_STAT_H)
  add_definitions(-DHAVE_SYS_STAT_H)
endif()

check_include_files(sys/times.h HAVE_SYS_TIMES_H)
if(HAVE_SYS_TIMES_H)
  add_definitions(-DHAVE_SYS_TIMES_H)
endif()

check_include_files(sys/types.h HAVE_SYS_TYPES_H)
if(HAVE_SYS_TYPES_H)
  add_definitions(-DHAVE_SYS_TYPES_H)
endif()

check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions(-DHAVE_UNISTD_H)
endif()

check_function_exists(getenv HAVE_GETENV)
if(HAVE_GETENV)
  add_definitions(-DHAVE_GETENV)
endif()

check_function_exists(getpagesize HAVE_GETPAGESIZE)
if(HAVE_GETPAGESIZE)
  add_definitions(-DHAVE_GETPAGESIZE)
endif()

check_function_exists(mmap HAVE_MMAP)
if(HAVE_MMAP)
  add_definitions(-DHAVE_MMAP)
endif()

check_function_exists(opendir HAVE_OPENDIR)
if(HAVE_OPENDIR)
  add_definitions(-DHAVE_OPENDIR)
endif()

check_function_exists(setjmp HAVE_SETJMP)
if(HAVE_SETJMP)
  add_definitions(-DHAVE_SETJMP)
endif()

check_function_exists(sqrt HAVE_SQRT)
if(HAVE_SQRT)
  add_definitions(-DHAVE_SQRT)
endif()

check_function_exists(strstr HAVE_STRSTR)
if(HAVE_STRSTR)
  add_definitions(-DHAVE_STRSTR)
endif()

find_library(HAVE_ICONV "iconv")
if(HAVE_ICONV)
  add_definitions(-DICONV_CONST)
endif()

find_library(HAVE_LIBM "m")
if(HAVE_LIBM)
  add_definitions(-DHAVE_LIBM)
endif()

add_definitions(-DHAVE_CONFIG_H
                -DDIC_VERSION=102
                -DMECAB_DEFAULT_RC="dummy"
                -DMECAB_WITHOUT_SHARE_DIC
                -DPACKAGE="open_jtalk"
                -DVERSION="${OPEN_JTALK_VERSION}")

# whether build as a shared library or not 
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
else()
  set(LIB_TYPE STATIC)
endif()

# find HTSEngine
find_path(HTS_ENGINE_INCLUDE_DIR HTS_engine.h)
find_library(HTS_ENGINE_LIB HTSEngine)
if(HTS_ENGINE_INCLUDE_DIR AND HTS_ENGINE_LIB)
  target_link_libraries(OpenJTalk ${HTS_ENGINE_LIB})
  include_directories(OpenJTalk ${HTS_ENGINE_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Required HTSEngine not found")
endif()

# configuration for charset
#option(CHARSET "Encoding set for mecab" "eucjp")
set(CHARSET "sjis")

if(${CHARSET} STREQUAL "sjis")
  add_definitions(-DCHARSET_SHIFT_JIS -DMECAB_CHARSET=sjis)
  foreach(flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    set(${flag} "${${flag}} -finput-charset=cp932 -fexec-charset=cp932")
  endforeach()
elseif(${CHARSET} STREQUAL "eucjp")
  add_definitions(-DCHARSET_EUC_JP -DMECAB_CHARSET=euc-jp)
elseif(${CHARSET} STREQUAL "utf8")
  add_definitions(-DCHARSET_UTF_8 -DMECAB_CHARSET=utf-8 -DMECAB_UTF8_USE_ONLY)
  foreach(flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    set(${flag} "${${flag}} -finput-charset=UTF-8 -fexec-charset=UTF-8")
  endforeach()
else()
  message(FATAL_ERROR "Encoding ${CHARSET} not recognized. You can set sjis/eucjp/utf8")
endif()

# installation
if(NOT MSVC)
  install(TARGETS OpenJTalk DESTINATION lib)
  install(FILES jpcommon/jpcommon.h mecab/src/mecab.h mecab2njd/mecab2njd.h njd/njd.h njd2jpcommon/njd2jpcommon.h
          njd_set_accent_phrase/njd_set_accent_phrase.h njd_set_accent_type/njd_set_accent_type.h 
          njd_set_digit/njd_set_digit.h njd_set_long_vowel/njd_set_long_vowel.h njd_set_pronunciation/njd_set_pronunciation.h 
          njd_set_unvoiced_vowel/njd_set_unvoiced_vowel.h text2mecab/text2mecab.h
          DESTINATION include/OpenJTalk)
endif()

