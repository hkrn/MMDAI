#
# libvpvl is no longer used
#

require File.dirname(__FILE__) + '/cmake.rb'

module Mmdai

class Vpvl < Thor
  include Build::CMake

  desc "build", "build libvpvl"
  method_options :flag => :boolean
  def build
    invoke_build
  end

  desc "clean", "delete built libvpvl libraries"
  def clean
    invoke_clean
  end

protected
  def get_build_options(build_type, extra_options)
    return {
      :vpvl_build_qt_renderer => false,
      :vpvl_enable_glsl => false,
      :vpvl_enable_nvidia_cg => false,
      :vpvl_enable_opencl => false,
      :vpvl_enable_project => false,
      :vpvl_link_assimp => false,
      :vpvl_link_qt => false,
      :vpvl_opengl_renderer => false
    }
  end

  def get_directory_name
    return "libvpvl"
  end

end

end
