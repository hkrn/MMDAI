require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Assimp < Thor
  include Build::CMake
  include VCS::SVN

  desc "debug", "build assimp for debug"
  method_options :flag => :boolean
  def debug
    checkout
    rewrite_cmake_file Regexp.compile("assimp\s+STATIC"), "assimp SHARED"
    invoke_build :debug
  end

  desc "release", "build assimp for release"
  method_options :flag => :boolean
  def release
    checkout
    rewrite_cmake_file Regexp.compile("assimp\s+SHARED"), "assimp STATIC"
    invoke_build :release
  end

  desc "flascc", "build assimp for flascc (treats as release)"
  method_options :flag => :boolean
  def flascc
    checkout
    invoke_build :flascc
  end

  desc "emscripten", "build bullet for emscripten (treats as release)"
  method_options :flag => :boolean
  def emscripten
    checkout
    invoke_build :emscripten
  end

  desc "clean", "delete built assimp libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "https://assimp.svn.sourceforge.net/svnroot/assimp/tags/2.0"
  end

  def get_directory_name
    "assimp-src"
  end

  def get_build_options(build_type, extra_options)
    return {
      :build_assimp_tools => false,
      :enable_boost_workaround => true,
      :assimp_enable_boost_workaround => true,
      :assimp_build_static_lib => true,
      :assimp_build_assimp_tools => false,
      :assimp_build_samples => false,
      :assimp_build_tests => false
    }
  end

private
  def rewrite_cmake_file(from, to)
    path = "#{checkout_path}/code/CMakeLists.txt"
    gsub_file path, from, to
  end

end

end
