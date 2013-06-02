require File.dirname(__FILE__) + '/base.rb'

module Mmdai

  module Build

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
        if is_darwin? and extra_options.key? :separated_build then
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
          cflags = extra_options[:extra_cflags] || []
          if is_darwin? then
            cflags.push [ "-arch", "i386", "-arch", "x86_64" ]
          end
          configure = "CFLAGS=\"#{cflags.join(' ')}\" CXXFLAGS=\"#{cflags.join(' ')}\" " + configure
          configure += " --prefix=#{build_directory}/#{INSTALL_ROOT_DIR}"
          inside build_directory do
            run configure
            make
            make "install"
          end
        end
      end

      def start_clean(build_directory, separated_arch = false)
        [ :debug, :release ].each do |build_type|
          build_directory = get_build_directory build_type
          if is_darwin? and separated_arch then
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
        "../configure"
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
        base_path = "#{File.dirname(__FILE__)}/../../#{get_directory_name}/build-"
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

  end

end
