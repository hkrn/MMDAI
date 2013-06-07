require File.dirname(__FILE__) + '/configure.rb'
require File.dirname(__FILE__) + '/svn.rb'

module Mmdai

class Glog < Thor
  include Build::Configure
  include VCS::SVN

  desc "debug", "build glog for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug
  end

  desc "release", "build glog for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release
  end

  desc "clean", "delete built glog libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "http://google-glog.googlecode.com/svn/trunk"
  end

  def get_directory_name
    "glog-src"
  end

  def get_build_options(build_type, extra_options)
    {
      :disable_rtti => nil,
      :disable_shared => nil,
      :enable_static => nil,
      :without_gflags => nil
    }
  end

  def get_debug_flag_for_configure
    ""
  end

end

end
