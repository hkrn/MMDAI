require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Bullet < Thor
  include Build::CMake
  include VCS::SVN

  desc "build", "build Bullet Physics"
  method_options :flag => :boolean
  def build
    checkout
    invoke_build
  end

  desc "clean", "delete Bullet Physics libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "http://bullet.googlecode.com/svn/tags/bullet-2.77"
  end

  def get_directory_name
    "bullet-src"
  end

  def get_build_options(build_type, extra_options)
    return {
      :build_demos => false,
      :build_extras => false,
      :install_libs => true,
      :use_glut => false
    }
  end

end

end

