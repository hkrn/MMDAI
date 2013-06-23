require File.dirname(__FILE__) + '/tools/thor/alsoft.rb'
require File.dirname(__FILE__) + '/tools/thor/alure.rb'
require File.dirname(__FILE__) + '/tools/thor/assimp.rb'
require File.dirname(__FILE__) + '/tools/thor/bullet.rb'
require File.dirname(__FILE__) + '/tools/thor/git.rb'
require File.dirname(__FILE__) + '/tools/thor/glew.rb'
require File.dirname(__FILE__) + '/tools/thor/glfw.rb'
require File.dirname(__FILE__) + '/tools/thor/gli.rb'
require File.dirname(__FILE__) + '/tools/thor/glm.rb'
require File.dirname(__FILE__) + '/tools/thor/glog.rb'
require File.dirname(__FILE__) + '/tools/thor/icu.rb'
require File.dirname(__FILE__) + '/tools/thor/libav.rb'
require File.dirname(__FILE__) + '/tools/thor/sdl2.rb'
require File.dirname(__FILE__) + '/tools/thor/tbb.rb'
require File.dirname(__FILE__) + '/tools/thor/tinyxml2.rb'
require File.dirname(__FILE__) + '/tools/thor/vpvl.rb'
require File.dirname(__FILE__) + '/tools/thor/vpvl2.rb'
require File.dirname(__FILE__) + '/tools/thor/vpvm.rb'
require File.dirname(__FILE__) + '/tools/thor/zlib.rb'

module Mmdai

  class All < Thor

    DEPENDENCIES = [
      "bullet",
      "assimp",
      # "nvtt", NVTT no longer will be used
      "libxml2",
      "zlib",
      "libav",
      "icu",
      "glog",
      "tbb",
      "alsoft",
      "alure",
      "vpvl2"
    ]

    desc "debug", "build libvpvl2 and dependencies for debug"
    method_options :flag => :boolean
    def debug
      invoke_all_to_build :debug
    end

    desc "release", "build libvpvl2 and dependencies for release"
    method_options :flag => :boolean
    def release
      invoke_all_to_build :release
    end

    desc "flascc", "build libvpvl2 and dependencies for flascc"
    method_options :flag => :boolean
    def flascc
      invoke_all_to_build :flascc
    end

    desc "emscripten", "build libvpvl2 and dependencies for emscripten"
    method_options :flag => :boolean
    def emscripten
      invoke_all_to_build :emscripten
    end

    desc "clean", "delete built libvpvl2 and dependencies"
    def clean
      invoke_all :clean
    end

  private
    def invoke_all_to_build(command_type)
      command = command_type.to_s
      invoke "mmdai:bullet:" + command
      invoke "mmdai:assimp:" + command
      if command_type != :flascc then
        invoke "mmdai:nvtt:" + command
      end
      if command_type != :flascc and command_type != :emscripten then
        invoke "mmdai:libxml2:" + command
        invoke "mmdai:zlib:" + command
        invoke "mmdai:glew:" + command
        invoke "mmdai:gli:" + command
        invoke "mmdai:glm:" + command
        invoke "mmdai:libav:" + command
        invoke "mmdai:icu:" + command
        invoke "mmdai:glog:" + command
        invoke "mmdai:tbb:" + command
        invoke "mmdai:alsoft:" + command
        invoke "mmdai:alure:" + command
      end
      invoke "mmdai:vpvl2:" + command
    end

    def invoke_all(command)
      DEPENDENCIES.each do |library|
        invoke "mmdai:#{library}:#{command.to_s}"
      end
    end

  end

end
