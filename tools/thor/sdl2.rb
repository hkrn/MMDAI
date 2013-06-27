require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Sdl2 < Thor
  include Build::CMake
  include VCS::Http

  desc "debug", "build glfw for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end

  desc "release", "build glfw for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release, :no_visibility_flags => true
  end

  desc "clean", "delete built glfw libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "http://www.libsdl.org/tmp/release/#{get_filename}"
  end

  def get_basename
    "SDL2-2.0.0"
  end

  def get_filename
    "SDL2-2.0.0.tar.gz"
  end

  def get_directory_name
    return "SDL2-src"
  end

  def get_build_options(build_type, extra_options)
    return {
      :directx => false
    }
  end

end

end
