require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Gtest < Thor
  include Build::Base
  include VCS::SVN

  desc "build", "build googletest (doesn't build actually)"
  def build
    checkout
  end

  desc "clean", "delete built googletest libraries (do nothing)"
  def clean
  end

protected
  def get_uri
    "http://googletest.googlecode.com/svn/tags/release-1.7.0"
  end

  def get_directory_name
    "gtest-src"
  end

end

end

