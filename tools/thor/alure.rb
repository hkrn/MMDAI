require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Alure < Thor
  include Build::CMake
  include VCS::Git

  desc "debug", "build ALURE for debug"
  method_options :flag => :boolean
  def debug
    checkout
    ENV["OPENALDIR"] = get_alsoft_directory :debug
    invoke_build :debug
  end

  desc "release", "build ALURE for release"
  method_options :flag => :boolean
  def release
    checkout
    ENV["OPENALDIR"] = get_alsoft_directory :release
    invoke_build :release
  end

  desc "clean", "delete built bullet libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "git://repo.or.cz/alure.git"
  end

  def get_directory_name
    "alure-src"
  end

  def get_tag_name
    "alure-1.2"
  end

  def get_build_options(build_type, extra_options)
    # force building ALURE as static to use OpenAL soft runtime instead of built-in OpenAL runtime on OSX
    return {
      :build_shared => false,
      :build_static => true,
      :dynload => false,
      :sndfile => false,
      :vorbis => false,
      :flac => false,
      :mpg123 => false,
      :dumb => false,
      :modplug => false,
      :fluidsynth => false,
      :build_examples => false,
      :install_examples => false
    }
  end

private
  def get_alsoft_directory(build_type)
    "#{File.dirname(__FILE__)}/../../openal-soft-src/build-#{build_type.to_s}/#{INSTALL_ROOT_DIR}"
  end

end

end
