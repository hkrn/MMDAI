require File.dirname(__FILE__) + '/cmake.rb'

module Mmdai

class Vpvl2 < Thor
  include Build::CMake

  desc "build", "build libvpvl2"
  method_options :flag => :boolean
  def build
    invoke_build
  end

  desc "clean", "delete built libvpvl2 libraries"
  def clean
    invoke_clean
  end

protected
  def get_build_options(build_type, extra_options)
    build_suite = (not need_opengl_es?)
    is_debug = (build_type === :debug)
    config = {
      :cmake_osx_deployment_target => "10.8",
      :vpvl2_build_qt_renderer => has_env_vars?([ "VPVL2_CMAKE_QT" ]),
      :vpvl2_enable_custom_release_clang => (build_suite and not is_debug),
      :vpvl2_enable_cxx11 => has_env_vars?([ "VPVL2_CMAKE_CXX11" ]),
      :vpvl2_enable_debug_annotations => has_env_vars?([ "VPVL2_CMAKE_ANNOTATIONS" ]),
      :vpvl2_enable_gles2 => need_opengl_es?,
      :vpvl2_enable_lazy_link => (build_type === :emscripten),
      :vpvl2_enable_nvidia_cg => false,
      :vpvl2_enable_opencl => (is_darwin? and build_suite) ? true : false,
      :vpvl2_enable_openmp => has_env_vars?([ "VPVL2_CMAKE_OPENMP" ]),
      :vpvl2_enable_extensions_archive => build_suite,
      :vpvl2_enable_extensions_project => true,
      :vpvl2_enable_extensions_applicationcontext => true,
      :vpvl2_enable_extensions_string => build_suite,
      :vpvl2_enable_extensions_world => true,
      :vpvl2_enable_lazy_link => false,
      :vpvl2_enable_test => (build_suite and is_debug and not is_msvc? and not is_darwin?),
      :vpvl2_link_assimp3 => build_suite,
      :vpvl2_link_atb => has_env_vars?([ "VPVL2_CMAKE_ATB" ]),
      :vpvl2_link_egl => is_msvc?,
      :vpvl2_link_glew => true,
      :vpvl2_link_glfw => has_env_vars?([ "VPVL2_CMAKE_GLFW" ]),
      :vpvl2_link_glog => has_env_vars?([ "VPVL2_CMAKE_GLOG" ]),
      :vpvl2_link_glslopt => true,
      :vpvl2_link_intel_tbb => build_suite,
      :vpvl2_link_nvfx => true,
      :vpvl2_link_qt => has_env_vars?([ "VPVL2_CMAKE_QT" ]),
      :vpvl2_link_regal => false,
      :vpvl2_link_sdl2 => has_env_vars?([ "VPVL2_CMAKE_SDL2" ]),
      :vpvl2_link_sfml => has_env_vars?([ "VPVL2_CMAKE_SFML" ]),
      :vpvl2_link_vpvl => false
    }
    return config
  end

  def get_directory_name
    return "libvpvl2"
  end

end

end
