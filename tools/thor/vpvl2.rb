require File.dirname(__FILE__) + '/cmake.rb'

module Mmdai

class Vpvl2 < Thor
  include Build::CMake

  desc "debug", "build libvpvl2 for debug"
  method_options :flag => :boolean
  def debug
    invoke_build :debug
  end

  desc "release", "build libvpvl2 for release"
  method_options :flag => :boolean
  def release
    invoke_build :release
  end

  desc "clean", "delete built libvpvl2 libraries"
  def clean
    invoke_clean
  end

protected
  def get_build_options(build_type, extra_options)
    # TODO: make render_type selectable by extra_options
    build_suite = false
    is_gles2 = false
    case build_type
    when :flascc then
    when :emscripten then
      is_gles2 = true
    else
      build_suite = true
    end
    is_debug = (build_type === :debug)
    is_assimp3 = ENV.key? "ASSIMP_V3"
    config = {
      :vpvl2_build_qt_renderer => is_debug,
      :vpvl2_enable_custom_release_clang => (not is_debug),
      :vpvl2_enable_gles2 => is_gles2,
      :vpvl2_enable_nvidia_cg => false,
      :vpvl2_enable_opencl => (is_darwin? and build_suite) ? true : false,
      :vpvl2_enable_openmp => false,
      :vpvl2_enable_extensions_archive => build_suite,
      :vpvl2_enable_extensions_project => build_suite,
      :vpvl2_enable_extensions_applicationcontext => build_suite,
      :vpvl2_enable_extensions_string => true,
      :vpvl2_enable_extensions_world => true,
      :vpvl2_enable_lazy_link => false,
      :vpvl2_enable_test => (build_suite and is_debug and not is_msvc?),
      :vpvl2_link_assimp => (build_suite and !is_assimp3),
      :vpvl2_link_assimp3 => (build_suite and is_assimp3),
      :vpvl2_link_atb => build_suite,
      :vpvl2_link_glew => build_suite,
      :vpvl2_link_glfw => (build_suite and is_debug),
      :vpvl2_link_glog => build_suite,
      :vpvl2_link_intel_tbb => build_suite,
      :vpvl2_link_nvfx => true,
      :vpvl2_link_qt => build_suite,
      :vpvl2_link_regal => true,
      :vpvl2_link_sdl2 => (build_suite and is_debug),
      :vpvl2_link_sfml => (build_suite and is_debug),
      :vpvl2_link_vpvl => false
    }
    return config
  end

  def get_directory_name
    return "libvpvl2"
  end

end

end

