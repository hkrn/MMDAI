#
# NVTT is no longer used
#

require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Nvtt < Thor
  include Build::CMake
  include VCS::SVN

  desc "build", "build NVTT"
  method_options :flag => :boolean
  def build
    checkout
    invoke_build
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

