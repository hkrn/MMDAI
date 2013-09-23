require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Alure < Thor
  include Build::CMake
  include VCS::Git

  desc "build", "build ALURE"
  method_options :flag => :boolean
  def build
    checkout
    ENV["OPENALDIR"] = get_alsoft_directory
    invoke_build
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
  def get_alsoft_directory()
    "#{File.dirname(__FILE__)}/../../openal-soft-src/#{get_build_directory}/#{INSTALL_ROOT_DIR}"
  end

end

end
