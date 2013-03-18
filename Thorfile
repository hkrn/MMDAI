require "rbconfig"
require "net/ftp"
require "net/https"

module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    module SVN
      include Thor::Actions
      def checkout
        if !options.key?("flag") then
          run "svn checkout #{get_uri} #{File.dirname(__FILE__)}/#{get_directory_name}"
        end
      end
    end # end of module SVN

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_tag_name
    module Git
      include Thor::Actions
      def checkout
        if !options.key?("flag") then
          base = "#{File.dirname(__FILE__)}/#{get_directory_name}"
          run "git clone #{get_uri} #{base}"
          inside base do
            run "git checkout #{get_tag_name}"
          end
        end
      end
    end # end of module Git

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_basename
    # * get_filename
    module Http
      include Thor::Actions

      def checkout
        if !options.key?("flag") then
          base = "#{File.dirname(__FILE__)}/#{get_directory_name}"
          if not File.directory? base then
            path = "#{File.dirname(__FILE__)}/#{get_filename}"
            fetch_from_remote URI.parse(get_uri), path
            run "tar xzf #{path}"
            FileUtils.move "#{File.dirname(__FILE__)}/#{get_basename}", base
            remove_file path
          end
        end
      end

    private

      def fetch_from_remote(uri, path)
        if uri.scheme === "http" or uri.scheme === "https" then
          fetch_http uri, path
        elsif uri.scheme === "ftp" then
          fetch_ftp uri, path
        else
          raise ArgumentError, "Only http(s) or ftp is supported: #{uri}"
        end
      end

      def fetch_ftp(uri, path)
        Net::FTP.open uri.host do |ftp|
          ftp.login
          ftp.passive = true
          ftp.chdir File.dirname uri.path
          ftp.list File.basename(uri.path, ".tar.gz") + ".*"
          ftp.getbinaryfile File.basename(uri.path), path
        end
      end

      def fetch_http(uri, path, limit = 5)
        raise ArgumentError, 'HTTP redirect too deep' if limit == 0
        http = Net::HTTP.new uri.host
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        response = http.get uri.path
        case response
        when Net::HTTPRedirection
          fetch_http URI.parse(response['location']), path, limit - 1
        else
          File.open path, "wb" do |writer|
            writer.write response.body
          end
        end
      end

    end # end of module Http

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
        if options.key?("flag") then
          print_build_options build_type, extra_options
          puts
        else
          build_directory = get_build_directory build_type
          build_options = get_build_options build_type, extra_options
          empty_directory build_directory
          start_build build_options, build_type, build_directory, extra_options
        end
      end

      def invoke_clean
        [ :debug, :release ].each do |build_type|
          build_directory = get_build_directory build_type
          start_clean build_directory
        end
      end

      def make(argument = nil)
        if argument == nil then
          argument = "-j4"
        end
        run "make #{argument}"
      end

      def ninja_or_make(argument = nil)
        if is_ninja? then
          run "ninja #{argument}"
        else
          make(argument)
        end
      end

      def is_ninja?
        return ENV.key?("NINJA_BUILD")
      end

      def is_msvc?
        return ENV.key?("VCINSTALLDIR")
      end

      def is_darwin?
        return /^darwin/.match RbConfig::CONFIG["target_os"]
      end

      def is_executable?
        return false
      end

    end # end of module Base

    # included class must implement below methods
    # * get_build_options
    # * get_directory_name
    # * get_arch_flag_for_configure
    # * get_debug_flag_for_configure
    # * run_msvc_build
    module Configure
      include Base

    protected
      def start_build(build_options, build_type, build_directory, extra_options)
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
              make "install"
            end
          end
        elsif is_msvc? then
          run_msvc_build build_options, build_type, build_directory, extra_options
        else
          configure += "--prefix=#{build_directory}/#{INSTALL_ROOT_DIR}"
          inside build_directory do
            run configure
            make
            make "install"
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
                make "clean"
                FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
              end
            end
          else
            inside build_directory do
              make "clean"
              FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
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
      def start_build(build_options, build_type, build_directory, extra_options)
        cmake = get_cmake build_options, build_type, build_directory, extra_options
        inside build_directory do
          run cmake
          ninja_or_make
          ninja_or_make "install"
        end
      end

      def start_clean(build_directory)
        inside build_directory do
          ninja_or_make "clean"
          FileUtils.rmtree [
            'CMakeCache.txt',
            'CMakeFiles',
            'cmake_install.cmake',
            'install_manifest.txt',
            'Makefile',
            INSTALL_ROOT_DIR
          ]
        end
      end

      def get_cmake(build_options, build_type, build_directory, extra_options)
        cmake = "cmake "
        is_debug = build_type === :debug
        build_options.merge!({
          :build_shared_libs => (is_debug and not is_msvc?),
          :cmake_build_type => (is_debug ? "Debug" : "Release"),
          :cmake_install_prefix => "#{build_directory}/#{INSTALL_ROOT_DIR}",
          :cmake_install_name_dir => "#{build_directory}/#{INSTALL_ROOT_DIR}/lib",
        })
        if build_type === :release and not is_msvc? then
          build_options[:cmake_cxx_flags] = "-fvisibility=hidden -fvisibility-inlines-hidden"
          if is_darwin? and not is_executable? then
            build_options[:cmake_osx_architectures] = "i386;x86_64"
          end
        elsif build_type === :flascc then
          build_options[:cmake_cxx_flags] = "-fno-rtti -O4"
        elsif build_type === :emscripten then
          emscripten_path = ENV['EMSCRIPTEN']
          cmake = "#{emscripten_path}/emconfigure cmake -DCMAKE_AR=#{emscripten_path}/emar "
        elsif is_executable? then
          build_options.delete :build_shared_libs
        else
          build_options[:library_output_path] = "#{build_directory}/lib"
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
        if is_ninja? then
          cmake += "-G Ninja "
        end
        cmake += ".."
        return cmake
      end

      def print_build_options(build_type, extra_options = {})
        puts get_cmake get_build_options(build_type, extra_options), build_type, nil, extra_options
      end

    end # end of module CMake

  end # end of module Build

  class Bullet < Thor
    include Build::CMake
    include VCS::SVN

    desc "debug", "build bullet for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end
    desc "release", "build bullet for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
    end

    desc "flascc", "build bullet for flascc (treats as release)"
    method_options :flag => :boolean
    def flascc
      checkout
      invoke_build :flascc
    end

    desc "emscripten", "build bullet for emscripten (treats as release)"
    method_options :flag => :boolean
    def emscripten
      checkout
      invoke_build :emscripten
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
        :install_libs => true
      }
    end

  end # end of Bullet

  class Assimp < Thor
    include Build::CMake
    include VCS::SVN

    desc "debug", "build assimp for debug"
    method_options :flag => :boolean
    def debug
      checkout
      rewrite_cmake_file Regexp.compile("assimp\s+STATIC"), "assimp SHARED"
      invoke_build :debug
    end

    desc "release", "build assimp for release"
    method_options :flag => :boolean
    def release
      checkout
      rewrite_cmake_file Regexp.compile("assimp\s+SHARED"), "assimp STATIC"
      invoke_build :release
    end

    desc "flascc", "build assimp for flascc (treats as release)"
    method_options :flag => :boolean
    def flascc
      checkout
      invoke_build :flascc
    end

    desc "emscripten", "build bullet for emscripten (treats as release)"
    method_options :flag => :boolean
    def emscripten
      checkout
      invoke_build :emscripten
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
      gsub_file path, from, to
    end

  end # end of Assimp

  class Libxml2 < Thor
    include Build::Configure
    include VCS::Http

    desc "debug", "build libxml2 for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end

    desc "release", "build libxml2 for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
      make_universal_binaries :release, true
    end

    # use customized build rule
    desc "clean", "delete built libxml2 libraries"
    def clean
      [ :debug, :release ].each do |build_type|
        build_directory = get_build_directory build_type
        inside build_directory do
          make "clean"
          FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
        end
      end
    end

  protected
    def get_uri
      "ftp://xmlsoft.org/libxml2/#{get_filename}"
    end

    def get_basename
      "libxml2-2.9.0"
    end

    def get_filename
      "#{get_basename}.tar.gz"
    end

    def get_directory_name
      return "libxml2-src"
    end

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
          :disable_static => nil
        })
      end
      return options
    end

    def get_arch_flag_for_configure(arch)
      case arch
      when :i386 then
        return "CC='clang -m32'"
      when :x86_64 then
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

    def run_msvc_build(build_options, build_type, build_directory, extra_options)
      path = "#{File.dirname(__FILE__)}/#{get_directory_name}/win32"
      inside path do
        enable_debug = build_type === :debug ? "yes" : "no"
        run "nmake /f Makefile.msvc clean"
        run "cscript configure.js compiler=msvc prefix=..\\build-#{build_type}\\install-root debug=#{enable_debug} ftp=no http=no html=no catalog=no docb=no iconv=no icu=no iso8859x=no zlib=no lzma=no"
        run "nmake /f Makefile.msvc"
        run "nmake /f Makefile.msvc install"
      end
    end

  end

  class Zlib < Thor
    include Build::CMake
    include VCS::Http

    desc "debug", "build zlib for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end

    desc "release", "build zlib for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
    end

    desc "clean", "delete built zlib libraries"
    def clean
      invoke_clean
    end

  protected
    def get_download_options
      "http://prdownloads.sourceforge.net/libpng/zlib-1.2.7.tar.gz?download"
    end

    def get_basename
      "zlib-1.2.7"
    end

    def get_filename
      "#{get_basename}.tar.gz"
    end

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
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end

    desc "release", "build NVTT for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
    end

    desc "flascc", "build NVTT for flascc (treats as release)"
    method_options :flag => :boolean
    def flascc
      checkout
      invoke_build :flascc
    end

    desc "emscripten", "build NVTT for emscripten (treats as release)"
    method_options :flag => :boolean
    def emscripten
      checkout
      invoke_build :emscripten
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
    include Build::Base
    include VCS::Http

    desc "debug", "build GLEW for debug"
    method_options :flag => :boolean
    def debug
      checkout
      build :debug, "debug"
    end

    desc "release", "build GLEW for release"
    method_options :flag => :boolean
    def release
      checkout
      build :release
    end

    desc "clean", "delete built GLEW libraries (do nothing)"
    def clean
    end

  protected
    def get_uri
      "https://sourceforge.net/projects/glew/files/glew/1.9.0/glew-1.9.0.tgz/download"
    end

    def get_basename
      "glew-1.9.0"
    end

    def get_filename
      "glew-1.9.0.tgz"
    end

    def get_directory_name
      return "glew-src"
    end

  private
    def build(build_type, make_type = nil)
      if !options["flag"] then
        base = "#{File.dirname(__FILE__)}/#{get_directory_name}"
        flags = "-arch i386 -arch x86_64"
        config_file_to_rewrite = "#{base}/config/Makefile.darwin"
        if build_type === :release then
          gsub_file config_file_to_rewrite, Regexp.compile("^CFLAGS.EXTRA\s*=\s*-dynamic"),
                                            "CFLAGS.EXTRA = #{flags} -dynamic"
          gsub_file config_file_to_rewrite, Regexp.compile("^LDFLAGS.EXTRA\s*=\s*$"),
                                            "LDFLAGS.EXTRA = #{flags}"
        else
          gsub_file config_file_to_rewrite, Regexp.compile("^CFLAGS.EXTRA\s*=\s*#{flags}\s*-dynamic"),
                                            "CFLAGS.EXTRA = -dynamic"
          gsub_file config_file_to_rewrite, Regexp.compile("^LDFLAGS.EXTRA\s*=\s*#{flags}$"),
                                            "LDFLAGS.EXTRA = "
        end
        install_dir = "#{base}/build-#{build_type.to_s}/#{INSTALL_ROOT_DIR}"
        inside base do
          ENV["GLEW_DEST"] = install_dir
          if is_darwin?
            # disable stripping to create universal binary correctly
            ENV["STRIP"] = ""
          end
          make "uninstall"
          make "clean"
          make make_type
          make "install"
        end
        # darwin cannot link GLEW (universalized) statically on release
        if build_type === :release and not is_darwin? then
          [ "lib", "lib64" ].each do |dir|
            [ "so" ].each do |extension|
              FileUtils.rmtree [ Dir.glob("#{install_dir}/#{dir}/libGLEW*.#{extension}*") ]
            end
          end
        end
      end
    end

  end # end of Glew

  class Gli < Thor
    include Build::Base
    include VCS::Git

    desc "debug", "build GLI for debug (doesn't build actually)"
    def debug
      checkout
    end

    desc "release", "build GLI for release (doesn't build actually)"
    def release
      checkout
    end

    desc "clean", "delete built GLM libraries (do nothing)"
    def clean
    end

  protected
    def get_uri
      return "git://github.com/g-truc/gli.git"
    end

    def get_directory_name
      return "gli-src"
    end

    def get_tag_name
      return "0.4.1.0"
    end

  end # end of Gli

  class Glm < Thor
    include Build::Base
    include VCS::Git

    desc "debug", "build GLM for debug (doesn't build actually)"
    def debug
      checkout
    end

    desc "release", "build GLM for release (doesn't build actually)"
    def release
      checkout
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

  end # end of Glm

  class Libav < Thor
    include Build::Configure
    include VCS::Git

    desc "debug", "build libav for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end

    desc "release", "build libav for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
      make_universal_binaries :release, false
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
      case arch
      when :i386 then
        return "--arch=i386 --cc='clang -m32'"
      when :x86_64 then
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
    include VCS::Http

    desc "debug", "build libICU for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end

    desc "release", "build libICU for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
    end

    # use customized build rule
    desc "clean", "delete built ICU libraries"
    def clean
      [ :debug, :release ].each do |build_type|
        build_directory = get_build_directory build_type
        inside build_directory do
          make "clean"
          FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
        end
      end
    end

  protected
    def get_uri
      "http://download.icu-project.org/files/icu4c/50.1.2/#{get_filename}"
    end

    def get_basename
      "icu"
    end

    def get_filename
      "icu4c-50_1_2-src.tgz"
    end

    # use customized build rule
    def start_build(build_options, build_type, build_directory, extra_options)
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
        make "install"
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

  class Alsoft < Thor
    include Build::CMake
    include VCS::Git

    desc "debug", "build OpenAL soft for debug"
    method_options :flag => :boolean
    def debug
      checkout
      invoke_build :debug
    end
    desc "release", "build OpenAL soft for release"
    method_options :flag => :boolean
    def release
      checkout
      invoke_build :release
    end

    desc "clean", "delete built bullet libraries"
    def clean
      invoke_clean
    end

  protected
    def get_uri
      return "git://repo.or.cz/openal-soft.git"
    end

    def get_directory_name
      return "openal-soft-src"
    end

    def get_tag_name
      return "openal-soft-1.15"
    end

    def get_build_options(build_type, extra_options)
      return {
        :alsoft_dlopen => false,
        :alsoft_utils => false,
        :alsoft_examples => false,
        :alsoft_config => false
      }
    end

  end # end of Alsoft

  class Alure < Thor
    include Build::CMake
    include VCS::Git

    desc "debug", "build ALURE for debug"
    method_options :flag => :boolean
    def debug
      checkout
      ENV["OPENALDIR"] = get_alsoft_directory :debug
      invoke_build :debug
    end
    desc "release", "build ALURE for release"
    method_options :flag => :boolean
    def release
      checkout
      ENV["OPENALDIR"] = get_alsoft_directory :release
      invoke_build :release
    end

    desc "clean", "delete built bullet libraries"
    def clean
      invoke_clean
    end

  protected
    def get_uri
      return "git://repo.or.cz/alure.git"
    end

    def get_directory_name
      return "alure-src"
    end

    def get_tag_name
      return "alure-1.2"
    end

    def get_build_options(build_type, extra_options)
      return {
        :dynload => false,
        :sndfile => false,
        :vorbis => false,
        :flac => false,
        :mpg123 => false,
        :dumb => false,
        :modplug => false,
        :fluidsynth => false,
        :build_examples => false,
        :install_examples => false
      }
    end

  private
    def get_alsoft_directory(build_type)
      "#{File.dirname(__FILE__)}/openal-soft-src/build-#{build_type.to_s}/#{INSTALL_ROOT_DIR}"
    end

  end # end of Alure

  class Vpvl < Thor
    include Build::CMake

    desc "debug", "build libvpvl for debug"
    method_options :flag => :boolean
    def debug
      invoke_build :debug
    end

    desc "release", "build libvpvl for release"
    method_options :flag => :boolean
    def release
      invoke_build :release
    end

    desc "flascc", "build libvpvl for flascc (treats as release)"
    method_options :flag => :boolean
    def flascc
      invoke_build :flascc
    end

    desc "emscripten", "build libvpvl for emscripten (treats as release)"
    method_options :flag => :boolean
    def emscripten
      invoke_build :emscripten
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
        renderer_type = :unknown
      when :emscripten then
        renderer_type = :unknown
        is_gles2 = true
      else
        build_suite = true
        renderer_type = :qt
      end
      config = {
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
        :vpvl2_link_glew => build_suite,
        :vpvl2_link_intel_tbb => build_suite,
        :vpvl2_link_nvtt => false,
      }
      case renderer_type
      when :sdl1 then
        config[:vpvl2_link_sdl1] = true
      when :sdl2 then
        config[:vpvl2_link_sdl2] = true
      when :sfml then
        config[:vpvl2_link_sfml] = true
      when :egl then
        config[:vpvl2_link_egl] = true
      when :qt then
        is_debug = (build_type === :debug)
        config[:vpvl2_link_qt] = true
        config[:vpvl2_enable_test] = (is_debug and not is_msvc?)
        config[:vpvl2_build_qt_renderer] = is_debug
      end
      return config
    end

    def get_directory_name
      return "libvpvl2"
    end

  end

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

  class All < Thor

    DEPENDENCIES = [
      "bullet",
      "assimp",
      # "nvtt", NVTT no longer will be used
      "libxml2",
      "zlib",
      "libav",
      "icu",
      "alsoft",
      "alure",
      "vpvl",
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
        invoke "mmdai:alsoft:" + command
        invoke "mmdai:alure:" + command
      end
      invoke "mmdai:vpvl:" + command
      invoke "mmdai:vpvl2:" + command
    end

    def invoke_all(command)
      DEPENDENCIES.each do |library|
        invoke "mmdai:#{library}:#{command.to_s}"
      end
    end

  end # end of Vpvl2

end # end of Mmdai

