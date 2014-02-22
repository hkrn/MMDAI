require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Gizmo < Thor
  include Build::CMake
  include VCS::Git

  desc "build", "build LibGizmo"
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
    "https://github.com/hkrn/LibGizmo.git"
  end

  def get_directory_name
    "libgizmo-src"
  end

  def get_tag_name
    "master"
  end

  def get_build_options(build_type, extra_options)
    return {}
  end

end

end

