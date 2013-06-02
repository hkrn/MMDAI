require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Zlib < Thor
  include Build::CMake
  include VCS::Http

  desc "debug", "build zlib for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug, :no_visibility_flags => true
  end

  desc "release", "build zlib for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release, :no_visibility_flags => true
  end

  desc "clean", "delete built zlib libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "http://prdownloads.sourceforge.net/libpng/zlib-1.2.7.tar.gz?download"
  end

  def get_basename
    "zlib-1.2.7"
  end

  def get_filename
    "#{get_basename}.tar.gz"
  end

  def get_build_options(build_type, extra_options)
    {}
  end

  def get_directory_name
    "zlib-src"
  end

end

end
