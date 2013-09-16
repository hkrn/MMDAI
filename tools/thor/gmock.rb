require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Gmock < Thor
  include Build::Base
  include VCS::SVN

  desc "build", "build googlemock (doesn't build actually)"
  def build
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

