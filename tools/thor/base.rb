require "rbconfig"

module Mmdai

  module Build

    module Base
      include Thor::Actions

      INSTALL_ROOT_DIR = "install-root"

      def get_base_directory()
        File.expand_path "#{File.dirname(__FILE__)}/../.."
      end

      def  get_build_type()
        type = ENV["VPVL2_BUILD_TYPE"]
        case type
          when "ANDROID" then :android
          when "DEBUG" then :debug
          else :release
        end
      end

      def get_build_directory()
        File.expand_path "#{get_base_directory}/#{get_directory_name}/build-#{get_build_type.to_s}"
      end

    protected
      def invoke_build(extra_options = {})
        if options.key? "flag" then
          print_build_options get_build_type, extra_options
          puts
        else
          build_directory = get_build_directory
          build_options = get_build_options get_build_type, extra_options
          empty_directory build_directory
          start_build build_options, build_directory, extra_options
        end
      end

      def invoke_clean(separated_arch = false)
        build_directory = get_build_directory
        start_clean build_directory, separated_arch
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
        return ENV.key? "NINJA_BUILD"
      end

      def is_msvc?
        return ENV.key? "VCINSTALLDIR"
      end

      def is_darwin?
        return /^darwin/.match RbConfig::CONFIG["target_os"]
      end

      def is_executable?
        return false
      end

    end # end of module Base

  end

end
