require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Tinyxml2 < Thor
  include Build::CMake
  include VCS::Git

  desc "debug", "build tinyxml2 for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end

  desc "release", "build tinyxml2 for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release, :no_visibility_flags => true
  end

  desc "clean", "delete built tinyxml2 libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "https://github.com/leethomason/tinyxml2"
  end

  def get_directory_name
    "tinyxml2-src"
  end

  def get_tag_name
    "master"
  end

  def get_build_options(build_type, extra_options)
    return {
      :build_static_libs => true
    }
  end

end

end
