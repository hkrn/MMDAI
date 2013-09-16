require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Glfw < Thor
  include Build::CMake
  include VCS::Git

  desc "build", "build GLFW"
  method_options :flag => :boolean
  def build
    checkout
    invoke_build
  end

  desc "clean", "delete built GLFW libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "https://github.com/glfw/glfw.git"
  end

  def get_directory_name
    "glfw-src"
  end

  def get_tag_name
    "3.0.1"
  end

  def get_build_options(build_type, extra_options)
    return {
      :glfw_build_examples => false,
      :glfw_build_tests => false,
      :glfw_install => true,
      :glfw_document_internals => false,
      :glfw_build_universal => true
    }
  end

end

end
