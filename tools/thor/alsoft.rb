require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Alsoft < Thor
  include Build::CMake
  include VCS::Git

  desc "debug", "build OpenAL soft for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end

  desc "release", "build OpenAL soft for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release
  end

  desc "clean", "delete built bullet libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "git://repo.or.cz/openal-soft.git"
  end

  def get_directory_name
    "openal-soft-src"
  end

  def get_tag_name
    "openal-soft-1.15"
  end

  def get_build_options(build_type, extra_options)
    return {
      :alsoft_dlopen => false,
      :alsoft_utils => false,
      :alsoft_examples => false,
      :alsoft_config => false
    }
  end

end

end

