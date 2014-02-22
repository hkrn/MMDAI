require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Anttweakbar < Thor
  include Build::Base
  include VCS::Git

  desc "build", "build AntTweakBar (doesn't build actually)"
  def build
    checkout
  end

  desc "clean", "delete built AntTweakBar libraries (do nothing)"
  def clean
  end

protected
  def get_uri
    "https://github.com/hkrn/AntTweakBar.git"
  end

  def get_directory_name
    "AntTweakBar-src"
  end

  def get_tag_name
    "master"
  end

end

end

