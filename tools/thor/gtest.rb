require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Gtest < Thor
  include Build::Base
  include VCS::SVN

  desc "debug", "build googletest for debug (doesn't build actually)"
  def debug
    checkout
  end

  desc "release", "build googletest for release (doesn't build actually)"
  def release
    checkout
  end

  desc "clean", "delete built googletest libraries (do nothing)"
  def clean
  end

protected
  def get_uri
    "http://googletest.googlecode.com/svn/tags/release-1.6.0"
  end

  def get_directory_name
    "gtest-src"
  end

end

end
