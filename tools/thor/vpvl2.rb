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
      :vpvl2_build_qt_renderer => is_debug,
      :vpvl2_enable_custom_release_clang => (build_suite and not is_debug),
      :vpvl2_enable_cxx11 => false,
      :vpvl2_enable_debug_annotations => is_debug,
      :vpvl2_enable_gles2 => need_opengl_es?,
      :vpvl2_enable_lazy_link => (build_type === :emscripten),
      :vpvl2_enable_nvidia_cg => false,
      :vpvl2_enable_opencl => (is_darwin? and build_suite) ? true : false,
      :vpvl2_enable_openmp => false,
      :vpvl2_enable_extensions_archive => build_suite,
      :vpvl2_enable_extensions_project => true,
      :vpvl2_enable_extensions_applicationcontext => true,
      :vpvl2_enable_extensions_string => build_suite,
      :vpvl2_enable_extensions_world => true,
      :vpvl2_enable_lazy_link => false,
      :vpvl2_enable_test => (build_suite and is_debug and not is_msvc?),
      :vpvl2_link_assimp3 => build_suite,
      :vpvl2_link_atb => false,
      :vpvl2_link_egl => is_msvc?,
      :vpvl2_link_glew => true,
      :vpvl2_link_glfw => (build_suite and is_debug),
      :vpvl2_link_glog => build_suite,
      :vpvl2_link_intel_tbb => build_suite,
      :vpvl2_link_nvfx => true,
      :vpvl2_link_qt => build_suite,
      :vpvl2_link_regal => false,
      :vpvl2_link_sdl2 => (build_suite and is_debug),
      :vpvl2_link_sfml => false, # (build_suite and is_debug),
      :vpvl2_link_vpvl => false
    }
    return config
  end

  def get_directory_name
    return "libvpvl2"
  end

end

end
