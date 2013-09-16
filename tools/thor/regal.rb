require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai
class Regal < Thor
  include Build::Base
  include VCS::Http

  desc "build", "build Regal"
  method_options :flag => :boolean
  def build
    checkout
    start_build
  end

  desc "clean", "delete built Regal libraries"
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
    "https://github.com/p3/regal.git"
  end

  def get_directory_name
    "regal-src"
  end

  def get_tag_name
    "master"
  end

private
  def start_build()
    if !options.key? "flag" then
      install_dir = "#{checkout_path}/build-#{get_build_type.to_s}/#{INSTALL_ROOT_DIR}"
      inside checkout_path do
        if is_msvc? then
          inside "#{checkout_path}/build/vc10" do
            run "msbuild glew.sln /t:build /p:configuration=#{get_build_type.to_s}"
          end
        else
          ENV["REGAL_DEST"] = install_dir
          make
          make "install"
        end
      end
    end
  end

end
end

