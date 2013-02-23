require "rbconfig"

module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    module SVN
      include Thor::Actions
      def checkout
        run "svn checkout #{get_uri} #{File.dirname(__FILE__)}/#{get_directory_name}"
      end
    end # end of module SVN

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_tag_name
    module Git
      include Thor::Actions
      def checkout
        run "git clone #{get_uri} #{File.dirname(__FILE__)}/#{get_directory_name}"
        inside get_directory_name do
          run "git checkout #{get_tag_name}"
        end
      end
    end # end of module Git

  end # end of module VCS

  module Build

    module Base
      include Thor::Actions
      INSTALL_ROOT_DIR = "install-root"
      def get_build_directory(build_type)
        "#{File.dirname(__FILE__)}/#{get_directory_name}/build-#{build_type.to_s}"
      end
    protected
      def invoke_build(build_type, extra_options = {})
        build_directory = get_build_directory build_type
        build_options = get_build_options build_type, extra_options
        empty_directory build_directory
        start_build build_options, build_type, extra_options, build_directory
      end
      def invoke_clean
        [ :debug, :release ].each do |build_type|
          build_directory = get_build_directory build_type
          start_clean build_directory
        end
      end
      def make
        run "make -j4"
      end
      def make_clean
        run "make clean"
      end
      def make_install
        run "make install"
      end
      def delete_files(targets)
        run "rm -rf #{targets.join(' ')}"
      end
      def is_darwin?
        return /^darwin/.match RbConfig::CONFIG["target_os"]
      end
    end # end of module Base

    # included class must implement below methods
    # * get_build_options
    # * get_directory_name
    # * get_arch_flag_for_configure
    # * get_debug_flag_for_configure
    module Configure
      include Base
    protected
      def start_build(build_options, build_type, extra_options, build_directory)
        configure = get_configure_string build_options, build_type
        if build_type === :release and is_darwin? then
          [:i386, :x86_64].each do |arch|
            arch_directory = "#{build_directory}_#{arch.to_s}"
            arch_configure = configure
            arch_configure += get_arch_flag_for_configure(arch)
            arch_configure += " --prefix=#{arch_directory}"
            inside arch_directory do
              run arch_configure
              make
              make_install
            end
          end
        else
          configure += "--prefix=#{build_directory}/#{INSTALL_ROOT_DIR}"
          inside build_directory do
            run configure
            make
            make_install
          end
        end
      end
      def start_clean(build_directory)
        [ :debug, :release ].each do |build_type|
          build_directory = get_build_directory build_type
          if build_type === :release and is_darwin? then
            [:i386, :x86_64].each do |arch|
              arch_directory = "#{build_directory}_#{arch.to_s}"
              inside arch_directory do
                make_clean
                delete_files [ 'Makefile', INSTALL_ROOT_DIR ]
              end
            end
          else
            inside build_directory do
              make_clean
              delete_files [ 'Makefile', INSTALL_ROOT_DIR ]
            end
          end
        end
      end
      def get_configure_path
        return "../configure"
      end
      def get_configure_string(build_options, build_type)
        configure = get_configure_path
        if build_type === :debug then
          configure += " " + get_debug_flag_for_configure
        end
        configure += " "
        return serialize_build_options(configure, build_options)
      end
      def serialize_build_options(configure, build_options)
        build_options.each do |key, value|
          if value.nil? then
            configure += "--#{key.to_s.gsub(/_/, "-")} "
          elsif value.is_a? Array
            value.each do |item|
              configure += "--#{key.to_s.gsub(/_/, "-")}=#{item} "
            end
          else
            configure += "--#{key.to_s.gsub(/_/, "-")}=#{value} "
          end
        end
        return configure
      end
      def make_universal_binaries(build_type, is_static)
        if not is_darwin? then
          return
        end
        base_path = "#{File.dirname(__FILE__)}/#{get_directory_name}/build-"
		    build_path = base_path + build_type.to_s
        i386_directory = "#{build_path}_i386/lib"
        x86_64_directory = "#{build_path}_x86_64/lib"
        native_directory = "#{build_path}/#{INSTALL_ROOT_DIR}/lib"
        empty_directory native_directory
        target = is_static ? "*.a" : "*.dylib"
        Dir.glob "#{i386_directory}/#{target}" do |library_path|
          library = File.basename(library_path)
          i386_library = [ i386_directory, library ].join('/')
          x86_64_library = [ x86_64_directory, library ].join('/')
          univ_library = [ native_directory, library ].join('/')
          run "lipo -create -output #{univ_library} -arch i386 #{i386_library} -arch x86_64 #{x86_64_library}"
        end
        Dir.glob "#{native_directory}/*.#{target}" do |library_path|
          File.unlink(library_path)
        end
        FileUtils.cp_r "#{build_path}_i386/include", "#{build_path}/#{INSTALL_ROOT_DIR}/include"
      end
      def print_build_options(build_type, extra_options = {})
        puts get_configure_string get_build_options(build_type, extra_options), build_type
      end
    end # end of module Configure

    # included class must implement below methods
    # * get_build_options
    # * get_directory_name
    module CMake
      include Base
    protected
      def start_build(build_options, build_type, extra_options, build_directory)
        cmake = get_cmake build_options, extra_options, build_type, build_directory
        inside build_directory do
          run cmake
          make
          make_install
        end
      end
      def start_clean(build_directory)
        inside build_directory do
          make_clean
          delete_files [
            'CMakeCache.txt',
            'CMakeFiles',
            'cmake_install.cmake',
            'install_manifest.txt',
            'Makefile',
            INSTALL_ROOT_DIR
          ]
        end
      end
      def get_cmake(build_options, extra_options, build_type, build_directory)
        cmake = "cmake "
        build_options.merge!({
          :build_shared_libs => false,
          :cmake_build_type => (build_type === :debug ? "Debug" : "Release"),
          :cmake_install_prefix => "#{build_directory}/#{INSTALL_ROOT_DIR}",
          :cmake_install_name_dir => "#{build_directory}/#{INSTALL_ROOT_DIR}/lib",
          :library_output_path => "#{build_directory}/lib"
        })
        if build_type === :release then
          build_options[:cmake_cxx_flags] = "-fvisibility=hidden -fvisibility-inlines-hidden"
          if is_darwin? then
            build_options[:cmake_osx_architectures] = "i386;x86_64"
          end
        elsif build_type === :flascc then
          build_options[:cmake_cxx_flags] = "-fno-rtti -O4"
        elsif build_type === :emscripten then
          emscripten_path = ENV['EMSCRIPTEN']
          cmake = "#{emscripten_path}/emconfigure cmake -DCMAKE_AR=#{emscripten_path}/emar "
        else
          build_options[:build_shared_libs] = true
        end
        return serialize_build_options cmake, build_options
      end
      def serialize_build_options(cmake, build_options)
        build_options.each do |key, value|
          cmake += "-D"
          cmake += key.to_s.upcase
          if !!value == value then
          cmake += ":BOOL="
            cmake += value ? "ON" : "OFF"
          else
            cmake += ":STRING='"
            cmake += value
            cmake += "'"
          end
          cmake += " "
        end
        cmake += ".."
        return cmake
      end
      def print_build_options(build_type, extra_options = {})
        puts get_cmake get_build_options(build_type, extra_options), build_type, extra_options, nil
      end
    end # end of module CMake

  end # end of module Build

  class Bullet < Thor
    include Build::CMake
    include VCS::SVN
    desc "debug", "build bullet for debug"
    def debug
      checkout
      invoke_build :debug
    end
    desc "release", "build bullet for release"
    def release
      checkout
      invoke_build :release
    end
    desc "flascc", "build bullet for flascc (treats as release)"
    def flascc
      checkout
      invoke_build :flascc
    end
    desc "emscripten", "build bullet for emscripten (treats as release)"
    def emscripten
      checkout
      invoke_build :emscripten
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built bullet libraries"
    def clean
      invoke_clean
    end
  protected
    def get_uri
      return "http://bullet.googlecode.com/svn/tags/bullet-2.77"
    end
    def get_directory_name
      return "bullet-src"
    end
    def get_build_options(build_type, extra_options)
      return {
        :build_demos => false,
        :build_extras => false,
        :build_opencl_demos => false,
        :build_cpu_demos => false
      }
    end
  end # end of Bullet

  class Assimp < Thor
    include Build::CMake
    include VCS::SVN
    desc "debug", "build assimp for debug"
    def debug
      checkout
      rewrite_cmake_file Regexp.compile("assimp\s+STATIC"), "assimp SHARED"
      invoke_build :debug
    end
    desc "release", "build assimp for release"
    def release
      checkout
      rewrite_cmake_file Regexp.compile("assimp\s+SHARED"), "assimp STATIC"
      invoke_build :release
    end
    desc "flascc", "build assimp for flascc (treats as release)"
    def flascc
      checkout
      invoke_build :flascc
    end
    desc "emscripten", "build bullet for emscripten (treats as release)"
    def emscripten
      checkout
      invoke_build :emscripten
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built assimp libraries"
    def clean
      invoke_clean
    end
  protected
    def get_uri
      return "https://assimp.svn.sourceforge.net/svnroot/assimp/tags/2.0"
    end
    def get_directory_name
      return "assimp-src"
    end
    def get_build_options(build_type, extra_options)
      return {
        :build_assimp_tools => false,
        :enable_boost_workaround => true,
      }
    end
  private
    def rewrite_cmake_file(from, to)
      path = "#{File.dirname(__FILE__)}/#{get_directory_name}/code/CMakeLists.txt"
      content = File.open(path, "rb").read.gsub(from, to)
      File.open(path, "wb").write(content)
    end
  end # end of Assimp

  class Libxml2 < Thor
    include Build::Configure
    desc "debug", "build libxml2 for debug"
    def debug
      invoke_build :debug
    end
    desc "release", "build libxml2 for release"
    def release
      invoke_build :release
      make_universal_binaries :release, true
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    # use customized build rule
    desc "clean", "delete built libxml2 libraries"
    def clean
      [ :debug, :release ].each do |build_type|
        build_directory = get_build_directory build_type
        inside build_directory do
          make_clean
          delete_files [ 'Makefile', INSTALL_ROOT_DIR ]
        end
      end
    end
  protected
    def get_build_options(build_type, extra_options)
      options = {
        :without_iconv => nil,
        :without_zlib => nil,
        :without_python => nil,
        :without_readline => nil,
        :without_ftp => nil,
        :without_html => nil,
        :without_http => nil,
        :without_c14n => nil,
        :without_threads => nil,
        :without_regexps => nil,
        :without_valid => nil,
        :without_xinclude => nil,
        :without_xptr => nil,
        :without_docbook => nil,
        :without_push => nil,
        :without_catalog => nil,
        :without_schematron => nil,
        :without_modules => nil
      }
      if build_type === :release then
        options.merge!({
          :disable_shared => nil,
          :enable_static => nil
        })
      else
        options.merge!({
          :enable_shared => nil,
          :disable_static => nil,
          :without_tree => nil,
          :without_xpath => nil
        })
      end
      return options
    end
    def get_arch_flag_for_configure(arch)
      if arch === :i386
        return "CC='clang -m32'"
      elsif arch === :x86_64
        return "CC='clang'"
      else
        return ""
      end
    end
    def get_configure_path
      return "../configure"
    end
    def get_debug_flag_for_configure
      return ""
    end
    def get_directory_name
      return "libxml2-src"
    end
  end

  class Zlib < Thor
    include Build::CMake
    desc "debug", "build zlib for debug"
    def debug
      invoke_build :debug
    end
    desc "release", "build zlib for release"
    def release
      invoke_build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built zlib libraries"
    def clean
      invoke_clean
    end
  protected
    def get_build_options(build_type, extra_options)
      return {}
    end
    def get_directory_name
      return "zlib-src"
    end
  end

  class Nvtt < Thor
    include Build::CMake
    include VCS::SVN
    desc "debug", "build NVTT for debug"
    def debug
      checkout
      invoke_build :debug
    end
    desc "release", "build NVTT for release"
    def release
      checkout
      invoke_build :release
    end
    desc "flascc", "build NVTT for flascc (treats as release)"
    def flascc
      checkout
      invoke_build :flascc
    end
    desc "emscripten", "build NVTT for emscripten (treats as release)"
    def emscripten
      checkout
      invoke_build :emscripten
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built NVTT libraries"
    def clean
      invoke_clean
    end
  protected
    def get_uri
      return "http://nvidia-texture-tools.googlecode.com/svn/trunk"
    end
    def get_build_options(build_type, extra_options)
      return {}
    end
    def get_directory_name
      return "nvtt-src"
    end
  end # end of Nvtt

  class Glew < Thor
    include VCS::Git
    desc "debug", "build GLEW for debug"
    def debug
      checkout
      inside get_directory_name do
        run "make debug"
      end
    end
    desc "release", "build GLEW for release"
    def release
      checkout
      inside get_directory_name do
        run "make"
      end
    end
    desc "clean", "delete built GLEW libraries (do nothing)"
    def clean
    end
  protected
    def get_uri
      return "git://glew.git.sourceforge.net/gitroot/glew/glew"
    end
    def get_directory_name
      return "glew-src"
    end
    def get_tag_name
      return "glew-1.9.0"
    end
  end # end of Glew

  class Glm < Thor
    include Build::Base
    include VCS::Git
    desc "debug", "build GLM for debug (doesn't build actually)"
    def debug
      checkout
      make_own :debug
    end
    desc "release", "build GLM for release (doesn't build actually)"
    def release
      checkout
      make_own :release
    end
    desc "clean", "delete built GLM libraries (do nothing)"
    def clean
    end
  protected
    def get_uri
      return "git://ogl-math.git.sourceforge.net/gitroot/ogl-math/ogl-math"
    end
    def get_directory_name
      return "glm-src"
    end
    def get_tag_name
      return "0.9.3.4"
    end
  private
    def make_own(build_type)
      inside get_directory_name do
        run "make extensions"
        run "make"
      end
    end
  end # end of Glm

  class Libav < Thor
    include Build::Configure
    include VCS::Git
    desc "debug", "build libav for debug"
    def debug
      checkout
      invoke_build :debug
    end
    desc "release", "build libav for release"
    def release
      checkout
      invoke_build :release
      make_universal_binaries :release, false
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built libav libraries"
    def clean
      invoke_clean
    end
  protected
    def get_uri
      return "git://git.libav.org/libav.git"
    end
    def get_directory_name
      return "libav-src"
    end
    def get_tag_name
      return "v9.1"
    end
    def get_arch_flag_for_configure(arch)
      if arch === :i386
        return "--arch=i386 --cc='clang -m32'"
      elsif arch === :x86_64
        return "--arch=x86_64 --cc=clang"
      else
        return ""
      end
    end
    def get_debug_flag_for_configure
      return "--enable-debug=3 --disable-optimizations"
    end
    def get_build_options(build_type, extra_options)
      return {
        :enable_shared => nil,
        :disable_static => nil,
        :disable_avconv => nil,
        :disable_avplay => nil,
        :disable_avprobe => nil,
        :disable_avserver => nil,
        :disable_network => nil,
        :disable_bzlib => nil,
        :disable_libfreetype => nil,
        :disable_libopenjpeg => nil,
        :disable_decoders => nil,
        :disable_decoder => ['h264'],
        :enable_decoder => ['flac', 'h264', 'pcm_s16le'], # add h264 decoder (unused in app) to prevent link error
        :disable_encoders => nil,
        :enable_encoder => ['png', 'pcm_s16le', 'utvideo'],
        :disable_parsers => nil,
        :disable_demuxers => nil,
        :enable_demuxer => ['aiff', 'flac', 'wav'],
        :disable_muxers => nil,
        :enable_muxer => ['avi', 'mov'],
        :disable_protocols => nil,
        :enable_protocol => ['file'],
        :disable_filters => nil,
        :disable_bsfs => nil,
        :disable_indevs => nil,
        :disable_outdevs => nil,
        :enable_zlib => nil
      }
    end
  end # end of Libav

  class Icu < Thor
    include Build::Configure
    desc "debug", "build libICU for debug"
    def debug
      invoke_build :debug
    end
    desc "release", "build libICU for release"
    def release
      invoke_build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    # use customized build rule
    desc "clean", "delete built ICU libraries"
    def clean
      [ :debug, :release ].each do |build_type|
        build_directory = get_build_directory build_type
        inside build_directory do
          make_clean
          delete_files [ 'Makefile', INSTALL_ROOT_DIR ]
        end
      end
    end
  protected
    # use customized build rule
    def start_build(build_options, build_type, extra_options, build_directory)
      configure = get_configure_string build_options, build_type
      flags = [
        "-DUCONFIG_NO_BREAK_ITERATION",
        "-DUCONFIG_NO_COLLATION",
        "-DUCONFIG_NO_FORMATTING",
        "-DUCONFIG_NO_TRANSLITERATION"
      ]
      cflags = flags.join ' '
      inside build_directory do
        run "CFLAGS=\"#{cflags}\" CXXFLAGS=\"#{cflags}\" " + configure
        make
        make_install
        if is_darwin? and build_type === :debug then
          inside "#{INSTALL_ROOT_DIR}/lib" do
            version = 50
            [ "data", "uc", "i18n" ].each do |name|
              run "install_name_tool -id `pwd`/libicu#{name}.#{version}.dylib libicu#{name}.dylib"
            end
            [ "uc", "i18n" ].each do |name|
              run "install_name_tool -change libicudata.#{version}.dylib `pwd`/libicudata.#{version}.dylib libicu#{name}.dylib"
            end
            run "install_name_tool -change libicuuc.#{version}.dylib `pwd`/libicuuc.#{version}.dylib libicui18n.dylib"
          end
        end
      end
    end
    def get_build_options(build_type, extra_options)
      options = {
        :disable_icuio => nil,
        :disable_layout => nil,
        :disable_extras => nil,
        :disable_tests => nil,
        :disable_samples => nil,
        :prefix => "#{get_build_directory build_type}/#{INSTALL_ROOT_DIR}"
      }
      if build_type === :release then
        options.merge!({
          :disable_shared => nil,
          :enable_static => nil
        })
      else
        options.merge!({
          :enable_shared => nil,
          :disable_static => nil
        })
      end
      return options
    end
    def get_configure_path
      return "../source/configure"
    end
    def get_debug_flag_for_configure
      return ""
    end
    def get_directory_name
      return "icu-src"
    end
  end

  class Vpvl < Thor
    include Build::CMake
    desc "debug", "build libvpvl for debug"
    def debug
      invoke_build :debug
    end
    desc "release", "build libvpvl for release"
    def release
      invoke_build :release
    end
    desc "flascc", "build libvpvl for flascc (treats as release)"
    def flascc
      invoke_build :flascc
    end
    desc "emscripten", "build libvpvl for emscripten (treats as release)"
    def emscripten
      invoke_build :emscripten
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
    desc "clean", "delete built libvpvl libraries"
    def clean
      invoke_clean
    end
  protected
    def get_build_options(build_type, extra_options)
      return {
        :vpvl_build_qt_renderer => false,
        :vpvl_enable_glsl => false,
        :vpvl_enable_nvidia_cg => false,
        :vpvl_enable_opencl => false,
        :vpvl_enable_project => false,
        :vpvl_link_assimp => false,
        :vpvl_link_qt => false,
        :vpvl_opengl_renderer => false
      }
    end
    def get_directory_name
      return "libvpvl"
    end
  end # end of Vpvl

  class Vpvl2 < Thor
    include Build::CMake
    desc "debug", "build libvpvl2 for debug"
    def debug
      invoke_build :debug
    end
    desc "release", "build libvpvl2 for release"
    def release
      invoke_build :release
    end
    desc "flascc", "build libvpvl2 for flascc (treats as release)"
    def flascc
      invoke_build :flascc
      inside get_build_directory(:flascc) do
        base_dir = File.dirname(__FILE__)
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
    def emscripten
      invoke_build :emscripten
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
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
      if build_type === :flascc then
        renderer_type = :unknown
      elsif build_type === :emscripten
        renderer_type = :unknown
        is_gles2 = true
      else
        build_suite = true
        renderer_type = :qt
      end
      return {
        :vpvl2_build_qt_renderer => (renderer_type === :qt and build_type === :debug),
        :vpvl2_enable_gles2 => is_gles2,
        :vpvl2_enable_nvidia_cg => build_suite,
        :vpvl2_enable_opencl => (is_darwin? and build_suite) ? true : false,
        :vpvl2_enable_openmp => false,
        :vpvl2_enable_extensions_archive=> build_suite,
        :vpvl2_enable_extensions_project => build_suite,
        :vpvl2_enable_extensions_rendercontext => build_suite,
        :vpvl2_enable_extensions_string => true,
        :vpvl2_enable_extensions_world => true,
        :vpvl2_link_assimp => true,
        :vpvl2_link_egl => renderer_type === :egl,
        :vpvl2_link_glew => build_suite,
        :vpvl2_link_intel_tbb => build_suite,
        :vpvl2_link_nvtt => build_suite,
        :vpvl2_link_qt => renderer_type === :qt,
        :vpvl2_link_sdl1 => renderer_type === :sdl1,
        :vpvl2_link_sdl2 => renderer_type === :sdl2,
        :vpvl2_link_sfml => renderer_type === :sfml
      }
    end
    def get_directory_name
      return "libvpvl2"
    end
  end

  class All < Thor
    DEPENDENCIES = [
      "bullet",
      "assimp",
      "nvtt",
      "libxml2",
      "zlib",
      "libav",
      "icu",
      "vpvl",
      "vpvl2"
    ]
    desc "debug", "build libvpvl2 and dependencies for debug"
    def debug
      invoke_all_to_build :debug
    end
    desc "release", "build libvpvl2 and dependencies for release"
    def release
      invoke_all_to_build :release
    end
    desc "flascc", "build libvpvl2 and dependencies for flascc"
    def flascc
      invoke_all_to_build :flascc
    end
    desc "emscripten", "build libvpvl2 and dependencies for emscripten"
    def emscripten
      invoke_all_to_build :emscripten
    end
    desc "flags_debug", "print built options of libvpvl2 and dependencies for debug"
    def flags_debug
      invoke_all_to_print_flags :debug
    end
    desc "flags_release", "print built options of libvpvl2 and dependencies for release"
    def flags_release
      invoke_all_to_print_flags :release
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
        invoke "mmdai:glm:" + command
        invoke "mmdai:libav:" + command
        invoke "mmdai:icu:" + command
      end
      invoke "mmdai:vpvl:" + command
      invoke "mmdai:vpvl2:" + command
    end
    def invoke_all_to_print_flags(command_type)
      command = command_type.to_s
      DEPENDENCIES.each do |library|
        puts "[#{library}]"
        invoke "mmdai:#{library}:flags_#{command}"
        puts
      end
    end
    def invoke_all(command)
      DEPENDENCIES.each do |library|
        invoke "mmdai:#{library}:#{command.to_s}"
      end
    end
  end # end of Vpvl2

end # end of Mmdai

