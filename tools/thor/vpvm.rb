require File.dirname(__FILE__) + '/cmake.rb'

module Mmdai

class Vpvm < Thor
  include Build::CMake

  desc "debug", "build VPVM for debug"
  method_options :flag => :boolean
  def debug
    invoke_build :debug
  end

  desc "release", "build VPVM for release"
  method_options :flag => :boolean
  def release
    invoke_build :release
  end

  desc "clean", "delete built VPVM"
  def clean
    invoke_clean
  end

protected
  def get_build_options(build_type, extra_options)
    return {
      :vpvm_enable_libav => (not is_msvc?)
    }
  end

  def get_directory_name
    return "VPVM"
  end

  def is_executable?
    return true
  end

end

end
