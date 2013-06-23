require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Gmock < Thor
  include Build::Base
  include VCS::SVN

  desc "debug", "build googlemock for debug (doesn't build actually)"
  def debug
    checkout
  end

  desc "release", "build googlemock for release (doesn't build actually)"
  def release
    checkout
  end

  desc "clean", "delete built googlemock libraries (do nothing)"
  def clean
  end

protected
  def get_uri
    "http://googlemock.googlecode.com/svn/tags/release-1.6.0"
  end

  def get_directory_name
    "gmock-src"
  end

end

end
