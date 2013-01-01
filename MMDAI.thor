require "rbconfig"

module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    module SVN
      include Thor::Actions
      def checkout
        run "svn checkout " + get_uri + " " + get_directory_name
      end
    end # end of module SVN

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_tag_name
    module Git
      include Thor::Actions
      def checkout
        run "git clone " + get_uri + " " + get_directory_name
        inside get_directory_name do
          run "git checkout " + get_tag_name
        end
      end
    end # end of module Git

  end # end of module VCS

  module Build

    module Base
      include Thor::Actions
    protected
      def build(build_type, extra_options = {})
        directory = get_directory_name + "/build-" + build_type.to_s
        empty_directory directory
        invoke_build_system get_build_options(build_type, extra_options), build_type, directory
      end
      def make
        run "make -j4"
      end
      def make_install
        run "make install"
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
      def invoke_build_system(build_options, build_type, directory)
        configure = get_configure build_options, build_type
        if build_type === :release and is_darwin? then
          [:i386, :x86_64].each do |arch|
            arch_directory = directory + "_" + arch.to_s
            arch_configure = configure
            arch_configure += get_arch_flag_for_configure(arch)
            arch_configure += " --prefix=" + File::expand_path(arch_directory)
            inside arch_directory do
              run arch_configure
              make
              make_install
            end
          end
        else
          configure += "--prefix=" + File::expand_path(directory + "_native")
          inside directory do
            run configure
            make
            make_install
          end
        end
      end
      def get_configure(build_options, build_type)
        configure = "../configure "
        if build_type === :debug then
          configure += get_debug_flag_for_configure
          configure += " "
        end
        build_options.each do |key, value|
          if value.nil? then
            configure += "--"
            configure += key.to_s.gsub(/_/, "-")
            configure += " "
          else
            value.each do |item|
              configure += "--"
              configure += key.to_s.gsub(/_/, "-")
              configure += "="
              configure += item
              configure += " "
            end
          end
        end
        return configure
      end
      def make_universal_binaries(build_type)
        base_path = File::expand_path(get_directory_name)
        i386_directory = base_path + "/" + build_type.to_s + "_i386/lib"
        x86_64_directory = base_path + "/" + build_type.to_s + "_x86_64/lib"
        native_directory = base_path + "/" + build_type.to_s + "_native/lib"
        empty_directory native_directory
        Dir.glob i386_directory + "/*.dylib" do |library_path|
          library = File.basename(library_path)
          i386_library = i386_directory + "/" + library
          x86_64_library = x86_64_directory + "/" + library
          univ_library = native_directory + "/" + library
          run "lipo -create -output " + univ_library + " -arch i386 " + i386_library + " -arch x86_64 " + x86_64_library
        end
      end
      def print_build_options(build_type, extra_options = {})
        puts get_configure get_build_options(build_type, extra_options), build_type
      end
    end # end of module Configure

    # included class must implement below methods
    # * get_build_options
    # * get_directory_name
    module CMake
      include Base
    protected
      def invoke_build_system(build_options, build_type, directory)
        cmake = get_cmake build_options, build_type
        inside directory do
          run cmake
          make
        end
      end
      def get_cmake(build_options, build_type)
        cmake = "cmake "
        build_options[:cmake_build_type] = build_type.to_s
        if build_type === :release then
          build_options.merge!({
            :build_shared_libs => false,
            :cmake_cxx_flags => "-fvisibility=hidden -fvisibility-inlines-hidden",
            :cmake_osx_architectures => "i386;x86_64"
          })
        else
          build_options[:build_shared_libs] = true
        end
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
      end
      def print_build_options(build_type, extra_options = {})
        puts get_cmake get_build_options(build_type, extra_options), build_type
      end
    end # end of module CMake

  end # end of module Build

  class Bullet < Thor
    include Build::CMake
    include VCS::SVN
    desc "debug", "build bullet for debug"
    def debug
      checkout
      build :debug
    end
    desc "release", "build bullet for release"
    def release
      checkout
      build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
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
      build :debug
    end
    desc "release", "build assimp for release"
    def release
      checkout
      build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
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
  end # end of Assimp

  class Nvtt < Thor
    include Build::CMake
    include VCS::SVN
    desc "debug", "build NVTT for debug"
    def debug
      checkout
      build :debug
    end
    desc "release", "build NVTT for release"
    def release
      checkout
      build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
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
    include VCS::Git
    desc "debug", "build GLM for debug (doesn't build actually)"
    def debug
      checkout
    end
    desc "release", "build GLM for release (doesn't build actually)"
    def release
      checkout
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
  end # end of Glm

  class Libav < Thor
    include Build::Configure
    include VCS::Git
    desc "debug", "build libav for debug"
    def debug
      checkout
      build :debug
    end
    desc "release", "build libav for release"
    def release
      checkout
      build :release
      make_universal_binaries :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
  protected
    def get_uri
      return "git://git.libav.org/libav.git"
    end
    def get_directory_name
      return "libav-src"
    end
    def get_tag_name
      return "v0.8.3"
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
        :disable_ffmpeg => nil,
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
        :enable_decoder => ['flac', 'pcm_s16le'],
        :disable_encoders => nil,
        :enable_encoder => ['png', 'pcm_s16le'],
        :disable_parsers => nil,
        :disable_demuxers => nil,
        :enable_demuxer => ['aiff', 'flac', 'wav'],
        :disable_muxers => nil,
        :enable_muxer => ['mov'],
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

  class Vpvl < Thor
    include Build::CMake
    desc "debug", "build libvpvl for debug"
    def debug
      build :debug
    end
    desc "release", "build libvpvl for release"
    def release
      build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
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
      build :debug
    end
    desc "release", "build libvpvl2 for release"
    def release
      build :release
    end
    desc "flags_debug", "print built options for debug"
    def flags_debug
      print_build_options :debug
    end
    desc "flags_release", "print built options for release"
    def flags_release
      print_build_options :release
    end
  protected
    def get_build_options(build_type, extra_options)
      # TODO: make render_type selectable by extra_options
      renderer_type = :qt
      return {
        :vpvl2_build_qt_renderer => (renderer_type === :qt and build_type === :debug),
        :vpvl2_enable_nvidia_cg => true,
        :vpvl2_enable_opencl => is_darwin? ? true : false,
        :vpvl2_enable_openmp => false,
        :vpvl2_enable_project => true,
        :vpvl2_link_assimp => true,
        :vpvl2_link_egl => false,
        :vpvl2_link_glew => true,
        :vpvl2_link_intel_tbb => true,
        :vpvl2_link_nvtt => true,
        :vpvl2_link_qt => renderer_type === :qt,
        :vpvl2_link_sdl1 => renderer_type === :sdl1,
        :vpvl2_link_sdl2 => renderer_type === :sdl2,
        :vpvl2_link_sfml => renderer_type === :sfml,
        :vpvl2_opengl_renderer => true
      }
    end
    def get_directory_name
      return "libvpvl2"
    end
  end

  class All < Thor
    desc "debug", "build libvpvl2 and dependencies for debug"
    def debug_all
      invoke_dependencies_to_build :debug
    end
    desc "release", "build libvpvl2 and dependencies for release"
    def release_all
      invoke_dependencies_to_build :release
    end
    desc "flags_debug", "print built options of libvpvl2 and dependencies for debug"
    def flags_debug_all
      invoke_dependencies_to_print_flags :debug
    end
    desc "flags_release", "print built options of libvpvl2 and dependencies for release"
    def flags_release_all
      invoke_dependencies_to_print_flags :release
    end
  private
    def invoke_dependencies_to_build(command_type)
      command = command_type.to_s
      invoke "mmdai:bullet:" + command
      invoke "mmdai:assimp:" + command
      invoke "mmdai:nvtt:" + command
      invoke "mmdai:glew:" + command
      invoke "mmdai:glm:" + command
      invoke "mmdai:libav:" + command
      invoke "mmdai:vpvl:" + command
      invoke "mmdai:vpvl2:" + command
    end
    def invoke_dependencies_to_print_flags(command_type)
      command = command_type.to_s
      puts "[bullet]"
      invoke "mmdai:bullet:flags_" + command
      puts
      puts "[assimp]"
      invoke "mmdai:assimp:flags_" + command
      puts
      puts "[nvtt]"
      invoke "mmdai:nvtt:flags_" + command
      puts
      puts "[libav]"
      invoke "mmdai:libav:flags_" + command
      puts
      puts "[vpvl]"
      invoke "mmdai:vpvl:flags_" + command
      puts
      puts "[vpvl2]"
      invoke "mmdai:vpvl2:flags_" + command
    end
  end # end of Vpvl2

end # end of Mmdai

