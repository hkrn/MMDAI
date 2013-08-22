require File.dirname(__FILE__) + '/cmake.rb'

module Mmdai

class Vpvl2 < Thor
  include Build::CMake

  desc "debug", "build libvpvl2 for debug"
  method_options :flag => :boolean
  def debug
    invoke_build :debug
  end

  desc "release", "build libvpvl2 for release"
  method_options :flag => :boolean
  def release
    invoke_build :release
  end

  desc "flascc", "build libvpvl2 for flascc (treats as release)"
  method_options :flag => :boolean
  def flascc
    invoke_build :flascc
    inside get_build_directory(:flascc) do
      base_dir = File.dirname(__FILE__) + '/../../'
      package_name = "com.github.mmdai"
      cxx_include_flags = "-Iinclude -I../include -I#{base_dir}/bullet-src/src"
      export_symbol_file = "exports.sym"
      FileUtils.cp "#{base_dir}/libvpvl2/src/swig/vpvl2.i", "."
      run "$FLASCC/usr/bin/swig -as3 -package #{package_name} -c++ -module vpvl2 -includeall -ignoremissing #{cxx_include_flags} vpvl2.i"
      run "java -jar $FLASCC/usr/lib/asc2.jar -import $FLASCC/usr/lib/builtin.abc -import $FLASCC/usr/lib/playerglobal.abc vpvl2.as"
      run "$FLASCC/usr/bin/g++ #{cxx_include_flags} -O4 -c vpvl2_wrap.cxx"
      FileUtils.cp "#{base_dir}/scripts/#{export_symbol_file}", export_symbol_file
      run "$FLASCC/usr/bin/nm vpvl2_wrap.o | grep ' T ' | ruby -ne \"STDIN.read.split(/\n/).each do |line| puts line.split(/\\s+/)[2].gsub(/^__/, '_') end\" >> #{export_symbol_file}"
      run <<EOS
$FLASCC/usr/bin/g++ -O4  ../src/swig/main.cc #{cxx_include_flags} -L#{base_dir}/bullet-src/build-flascc/lib \
-L#{base_dir}/assimp-src/build-flascc/lib -L#{base_dir}/libvpvl/build-flascc/lib \
-L#{base_dir}/libvpvl2/build-flascc/lib vpvl2_wrap.o vpvl2.abc \
-Wl,--start-group \
-lvpvl2 -lvpvl -lassimp -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath \
-Wl,--end-group \
-pthread -emit-swc=#{package_name} -O4 -flto-api=#{export_symbol_file} -o vpvl2.swc
EOS
    end
  end

  desc "emscripten", "build libvpvl2 for emscripten (treats as release)"
  method_options :flag => :boolean
  def emscripten
    invoke_build :emscripten
  end

  desc "clean", "delete built libvpvl2 libraries"
  def clean
    invoke_clean
  end

protected
  def get_build_options(build_type, extra_options)
    # TODO: make render_type selectable by extra_options
    build_suite = false
    is_gles2 = false
    case build_type
    when :flascc then
    when :emscripten then
      is_gles2 = true
    else
      build_suite = true
    end
    is_debug = (build_type === :debug)
    is_assimp3 = ENV.key? "ASSIMP_V3"
    config = {
      :vpvl2_build_qt_renderer => is_debug,
      :vpvl2_enable_custom_release_clang => (not is_debug),
      :vpvl2_enable_gles2 => is_gles2,
      :vpvl2_enable_nvidia_cg => false,
      :vpvl2_enable_opencl => (is_darwin? and build_suite) ? true : false,
      :vpvl2_enable_openmp => false,
      :vpvl2_enable_extensions_archive => build_suite,
      :vpvl2_enable_extensions_project => build_suite,
      :vpvl2_enable_extensions_applicationcontext => build_suite,
      :vpvl2_enable_extensions_string => true,
      :vpvl2_enable_extensions_world => true,
      :vpvl2_enable_lazy_link => false,
      :vpvl2_enable_test => (build_suite and is_debug and not is_msvc?),
      :vpvl2_link_assimp => (build_suite and !is_assimp3),
      :vpvl2_link_assimp3 => (build_suite and is_assimp3),
      :vpvl2_link_atb => build_suite,
      :vpvl2_link_glew => build_suite,
      :vpvl2_link_glfw => (build_suite and is_debug),
      :vpvl2_link_glog => build_suite,
      :vpvl2_link_intel_tbb => build_suite,
      :vpvl2_link_nvfx => true,
      :vpvl2_link_qt => build_suite,
      :vpvl2_link_regal => true,
      :vpvl2_link_sdl2 => (build_suite and is_debug),
      :vpvl2_link_sfml => (build_suite and is_debug),
      :vpvl2_link_vpvl => false
    }
    return config
  end

  def get_directory_name
    return "libvpvl2"
  end

end

end

