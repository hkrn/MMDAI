require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Sfml2 < Thor
  include Build::CMake
  include VCS::Git

  desc "build", "build SFML2"
  method_options :flag => :boolean
  def build
    checkout
    invoke_build
  end

  desc "clean", "delete built SFML2 libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "https://github.com/LaurentGomila/SFML.git"
  end

  def get_directory_name
    "SFML2-src"
  end

  def get_tag_name
    "2.0"
  end

  def get_build_options(build_type, extra_options)
    return {
      :cmake_install_framework_prefix => "#{get_build_directory}/#{INSTALL_ROOT_DIR}/lib"
    }
  end

end

end
