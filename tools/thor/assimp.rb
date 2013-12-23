require File.dirname(__FILE__) + '/cmake.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Assimp < Thor
  include Build::CMake
  include VCS::Git

  desc "build", "build assimp"
  method_options :flag => :boolean
  def build
    extra_options = {}
    if is_msvc? then
      extra_options[:force_release] = true
    end
    checkout
    invoke_build extra_options
  end

  desc "clean", "delete built assimp libraries"
  def clean
    invoke_clean
  end

protected
  def get_uri
    "https://github.com/assimp/assimp.git"
  end

  def get_directory_name
    "assimp-src"
  end

  def get_tag_name
    "master"
  end

  def get_build_options(build_type, extra_options)
    return {
      :assimp_enable_boost_workaround => true,
      :assimp_build_static_lib => (build_type === :release),
      :assimp_build_assimp_tools => false,
      :assimp_build_samples => false,
      :assimp_build_tests => false
    }
  end

end

end
