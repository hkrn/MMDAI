require File.dirname(__FILE__) + '/base.rb'

module Mmdai

  module Build

    # included class must implement below methods
    # * get_build_options
    # * get_directory_name
    module CMake
      include Base

    protected
      def start_build(build_options, extra_options)
        cmake = get_cmake build_options, extra_options
        inside get_build_path do
          run cmake
          ninja_or_make
          ninja_or_make "install"
        end
      end

      def start_clean(arch = false)
        inside get_build_path do
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

      def get_cmake_executable()
        return find_env_vars [ "VPVL2_CMAKE_EXECUTABLE", "CMAKE_EXECUTABLE" ], "cmake"
      end

      def get_cmake(build_options, extra_options)
        cmake = get_cmake_executable
        cmake += " "
        build_path = get_build_path
        build_type = get_build_type
        is_debug = build_type === :debug
        build_options.merge!({
          :build_shared_libs => (is_debug and not is_msvc?),
          :cmake_build_type => (is_debug ? "Debug" : "Release"),
          :cmake_c_flags => "",
          :cmake_cxx_flags => "",
          :cmake_install_prefix => "#{build_path}/#{INSTALL_ROOT_DIR}",
          :cmake_install_name_dir => "#{build_path}/#{INSTALL_ROOT_DIR}/lib",
        })
        if build_type === :release and !extra_options.key? :no_visibility_flags and not is_msvc? then
          build_options[:cmake_cxx_flags] += "-fvisibility=hidden -fvisibility-inlines-hidden"
        elsif build_type === :flascc then
          add_cflags "-fno-rtti -O4", build_options
        elsif build_type === :emscripten then
          emscripten_path = ENV['EMSCRIPTEN']
          cmake = "#{emscripten_path}/emconfigure cmake -DCMAKE_AR=#{emscripten_path}/emar "
        elsif is_executable? then
          build_options.delete :build_shared_libs
        else
          build_options[:library_output_path] = "#{build_path}/lib"
        end
        if is_darwin? and build_type === :release then
          add_cflags " -F/Library/Frameworks -mmacosx-version-min=10.5", build_options
          build_options[:cmake_osx_architectures] = "i386;x86_64"
        end
        return serialize_build_options cmake, build_options, extra_options.key?(:printable)
      end

      def serialize_build_options(cmake, build_options, is_printable)
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
          cmake += is_printable ? " \\\n" : " "
        end
        if is_ninja? then
          cmake += "-G Ninja "
        end
        cmake += ".."
        return cmake
      end

      def print_build_options(extra_options = {})
        extra_options[:printable] = true
        puts get_cmake get_build_options(get_build_type, extra_options), nil, extra_options
      end

      def add_cflags(cflags, build_options)
        build_options[:cmake_c_flags] += cflags
        build_options[:cmake_cxx_flags] += cflags
      end

    end

  end

end

