cmake_minimum_required(VERSION 2.8)

project(ql4pmx)
set(QL2PMX_MAJOR_VERSION 0)
set(QL2PMX_COMPAT_VERSION 1)
set(QL2PMX_MINOR_VERSION 0)
set(QL2PMX_VERSION "${QL2PMX_MAJOR_VERSION}.${QL2PMX_COMPAT_VERSION}.${QL2PMX_MINOR_VERSION}")

if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif()

function(get_source_directory output dir)
  set(${output} "${CMAKE_CURRENT_SOURCE_DIR}/../../${dir}" PARENT_SCOPE)
endfunction()

function(get_install_directory output dir)
  get_source_directory(source_dir ${dir})
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  set(build_dir "build-${CMAKE_BUILD_TYPE_TOLOWER}")
  set(${output} "${source_dir}/${build_dir}/install-root" PARENT_SCOPE)
endfunction()

function(link_dependencies target)
  target_link_libraries(${target} ${BULLET_LINEARMATH_LIB}
                                  ${BULLET_COLLISION_LIB}
                                  ${BULLET_DYNAMICS_LIB}
                                  ${BULLET_MULTITHREADED_LIB}
                                  ${BULLET_SOFTBODY_LIB}
                                  ${ASSIMP_LIBRARY}
                                  ${NVTT_NVCORE_LIBRARY}
                                  ${NVTT_NVMATH_LIBRARY}
                                  ${NVTT_NVIMAGE_LIBRARY}
                                  ${ICU_LIBRARY_DATA}
                                  ${ICU_LIBRARY_UC}
                                  ${ICU_LIBRARY_I18N}
                                  ${MESA3D_MESA_LIBRARY}
                                  ${MESA3D_OSMESA_LIBRARY}
                                  ${VPVL_LIBRARY}
                                  ${VPVL2_LIBRARY}
                                  ${FOUNDATION_FRAMEWORK}
                                  ${COCOA_FRAMEWORK})
endfunction()

get_install_directory(BULLET_INSTALL_DIR "bullet-src")
find_path(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h PATH_SUFFIXES include/bullet PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(BULLET_LINEARMATH_LIB LinearMath PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(BULLET_COLLISION_LIB BulletCollision PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(BULLET_DYNAMICS_LIB BulletDynamics PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(BULLET_MULTITHREADED_LIB BulletMultiThreaded PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATH_SUFFIXES lib64 lib32 lib PATHS ${BULLET_INSTALL_DIR} NO_DEFAULT_PATH)

get_install_directory(ASSIMP_INSTALL_DIR "assimp-src")
find_path(ASSIMP_INCLUDE_DIR NAMES assimp.h PATH_SUFFIXES include/assimp PATHS ${ASSIMP_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(ASSIMP_LIBRARY assimp PATH_SUFFIXES lib64 lib32 lib PATHS ${ASSIMP_INSTALL_DIR} NO_DEFAULT_PATH)

get_source_directory(NVTT_SOURCE_DIRECTORY "nvtt-src")
set(NVTT_INCLUDE_DIRECTORY "${NVTT_SOURCE_DIRECTORY}/src")
set(NVTT_BUILD_DIERCTORY "${NVTT_SOURCE_DIRECTORY}/build-${CMAKE_BUILD_TYPE}")
find_path(NVTT_CONFIG_INCLUDE_DIR nvconfig.h PATH_SUFFIXES src PATHS ${NVTT_BUILD_DIERCTORY} NO_DEFAULT_PATH)
find_path(NVTT_BASE_INCLUDE_DIR nvcore/nvcore.h PATHS ${NVTT_INCLUDE_DIRECTORY} NO_DEFAULT_PATH)
find_path(NVTT_POSH_INCLUDE_DIR posh.h PATH_SUFFIXES "extern/poshlib" PATHS ${NVTT_SOURCE_DIRECTORY} NO_DEFAULT_PATH)
include_directories(${NVTT_CONFIG_INCLUDE_DIR} ${NVTT_BASE_INCLUDE_DIR} ${NVTT_POSH_INCLUDE_DIR})
get_install_directory(NVTT_INSTALL_DIR "nvtt-src")
find_library(NVTT_NVCORE_LIBRARY nvcore PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(NVTT_NVMATH_LIBRARY nvimage PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(NVTT_NVIMAGE_LIBRARY nvimage PATH_SUFFIXES lib64 lib32 lib PATHS ${NVTT_INSTALL_DIR} NO_DEFAULT_PATH)

get_source_directory(GLM_SOURCE_DIR "glm-src")
find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${GLM_SOURCE_DIR} NO_DEFAULT_PATH)

get_install_directory(ICU_INSTALL_DIR "icu-src")
find_path(ICU_INCLUDE_DIR unicode/unistr.h PATH_SUFFIXES include PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(ICU_LIBRARY_DATA icudata icudt PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(ICU_LIBRARY_UC icuuc icuuc PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(ICU_LIBRARY_I18N icui18n icuin PATH_SUFFIXES lib64 lib32 lib PATHS ${ICU_INSTALL_DIR} NO_DEFAULT_PATH)

get_install_directory(VPVL_INSTALL_DIR "libvpvl")
find_path(VPVL_INCLUDE_DIR NAMES vpvl/vpvl.h PATH_SUFFIXES include PATHS ${VPVL_INSTALL_DIR} NO_DEFAULT_PATH)
find_library(VPVL_LIBRARY vpvl PATH_SUFFIXES lib64 lib32 lib PATHS ${VPVL_INSTALL_DIR} NO_DEFAULT_PATH)

get_source_directory(VPVL2_SOURCE_DIR "libvpvl2")
find_path(VPVL2_CONFIG_DIR NAMES vpvl2/config.h PATH_SUFFIXES "build-xcode/include" PATHS ${VPVL2_SOURCE_DIR} NO_DEFAULT_PATH)
find_path(VPVL2_INCLUDE_DIR NAMES vpvl2/vpvl2.h PATH_SUFFIXES include PATHS ${VPVL2_SOURCE_DIR} NO_DEFAULT_PATH)
find_library(VPVL2_LIBRARY vpvl2 PATH_SUFFIXES "build-xcode/lib/${CMAKE_BUILD_TYPE}" PATHS ${VPVL2_SOURCE_DIR} NO_DEFAULT_PATH)

get_source_directory(MESA3D_SOURCE_DIR "mesa-src")
find_library(MESA3D_MESA_LIBRARY mesa PATH_SUFFIXES "embed-darwin-x86_64/mesa" "darwin-x86_64/mesa" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
find_library(MESA3D_OSMESA_LIBRARY osmesa PATH_SUFFIXES "embed-darwin-x86_64/mesa/drivers/osmesa" "darwin-x86_64/mesa/drivers/osmesa" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
find_path(MESA3D_INCLUDE_DIR NAMES GL/osmesa.h PATH_SUFFIXES include PATHS ${MESA3D_SOURCE_DIR} NO_DEFAULT_PATH)

find_library(FOUNDATION_FRAMEWORK Foundation)
find_library(COCOA_FRAMEWORK COCOA)
find_library(QUICKLOOK_FRAMEWORK QuickLook)

include_directories(${BULLET_INCLUDE_DIR}
                    ${ASSIMP_INCLUDE_DIR}
                    ${GLM_INCLUDE_DIR}
                    ${ICU_INCLUDE_DIR}
                    ${MESA3D_INCLUDE_DIR}
                    ${VPVL_INCLUDE_DIR}
                    ${VPVL2_CONFIG_DIR}
                    ${VPVL2_INCLUDE_DIR}
                    "${CMAKE_CURRENT_SOURCE_DIR}/include")

aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/common" ql4pmx_common)
aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/ql4pmx" ql4pmx_sources)
file(GLOB "${CMAKE_CURRENT_SOURCE_DIR}/include/*" ql4pmx_headers)
add_library(ql4pmx MODULE ${ql4pmx_sources} ${ql4pmx_common} ${ql4pmx_headers})
set_target_properties(ql4pmx PROPERTIES MACOSX_BUNDLE TRUE
                                        BUNDLE TRUE
                                        BUNDLE_EXTENSION "qlgenerator"
                                        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist"
                                        MACOSX_BUNDLE_BUNDLE_NAME "ql4pmx"
                                        MACOSX_BUNDLE_SHORT_VERSION_STRING ${QL2PMX_VERSION}
                                        MACOSX_BUNDLE_VERSION ${QL2PMX_VERSION})
link_dependencies(ql4pmx)
target_link_libraries(ql4pmx ${QUICKLOOK_FRAMEWORK})

aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/ql4test" ql4test_sources)
add_executable(ql4test ${ql4test_sources} ${ql4pmx_common} ${ql4pmx_headers})
link_dependencies(ql4test)
