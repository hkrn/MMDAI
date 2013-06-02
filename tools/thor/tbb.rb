require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Tbb < Thor
  include Build::Base
  include VCS::Http

  desc "debug", "build TBB for debug"
  method_options :flag => :boolean
  def debug
    checkout
    start_build :debug
  end

  desc "release", "build GLEW for release"
  method_options :flag => :boolean
  def release
    checkout
    start_build :release
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
    "http://threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb41_20130314oss_src.tgz"
  end

  def get_basename
    "tbb41_20130314oss"
  end

  def get_filename
    "tbb41_20130314oss_src.tgz"
  end

  def get_directory_name
    "tbb-src"
  end

private
  def start_build(build_type, make_type = nil)
    inside checkout_path do
      name_rule = {
        :debug => "libtbb_debug",
        :release => "libtbb"
      }
      if is_msvc? then
        # FIXME: MSVC
      elsif is_darwin? then
        built_path = {}
        [ :ia32, :intel64 ].each do |arch|
          ENV["arch"] = arch.to_s
          ENV["cfg"] = "release"
          run "make"
          built_path[arch] = get_built_path checkout_path, build_type
          ENV.delete "arch"
          ENV.delete "cfg"
        end
        name = name_rule[build_type]
        product_path = "#{checkout_path}/lib/#{name}.dylib"
        empty_directory "#{checkout_path}/lib"
        run "lipo -create -output #{product_path} -arch i386 #{built_path[:ia32]}/#{name}.dylib -arch x86_64 #{built_path[:intel64]}/#{name}.dylib"
        run "install_name_tool -id #{product_path} #{product_path}"
      else
        ENV["cfg"] = "release"
        run "make"
        empty_directory "#{checkout_path}/lib"
        FileUtils.cp "#{get_built_path checkout_path, build_type}/#{name_rule[build_type]}.so", "#{checkout_path}/lib"
        ENV.delete "cfg"
      end
    end
  end

  def get_built_path(base, build_type)
    result = run("make info", :capture => true).split "\n"
    result.delete_at 0
    config = {}
    result.each do |item|
      pair = item.split "="
      config[pair[0]] = pair[1]
    end
    path = base + "/build/" + config["tbb_build_prefix"] + "_" + build_type.to_s
    return path
  end

end

end

