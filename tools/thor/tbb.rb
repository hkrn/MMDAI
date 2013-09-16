require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Tbb < Thor
  include Build::Base
  include VCS::Http

  desc "build", "build TBB for debug"
  method_options :flag => :boolean
  def build
    checkout
    start_build
  end

  desc "clean", "delete built TBB libraries"
  def clean
    if is_msvc? then
      inside "#{checkout_path}/build/vc10" do
        run "msbuild glew.sln /t:clean"
      end
    else
      inside checkout_path do
        make "clean"
      end
    end
  end

protected
  def get_uri
    "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb42_20130725oss_src.tgz"
  end

  def get_basename
    "tbb42_20130725oss"
  end

  def get_filename
    "tbb42_20130725oss_src.tgz"
  end

  def get_directory_name
    "tbb-src"
  end

private
  def start_build()
    inside checkout_path do
      if is_msvc? then
        # FIXME: MSVC
      else
        ENV["cfg"] = "release"
        run "make"
        ENV.delete "cfg"
      end
    end
  end

end

end

