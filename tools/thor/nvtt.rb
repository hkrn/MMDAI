require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Nvtt < Thor
  include Build::CMake
  include VCS::SVN

  desc "debug", "build NVTT for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end

  desc "release", "build NVTT for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release
  end

  desc "clean", "delete built NVTT libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "http://nvidia-texture-tools.googlecode.com/svn/trunk"
  end

  def get_build_options(build_type, extra_options)
    {}
  end

  def get_directory_name
    "nvtt-src"
  end

end

end
