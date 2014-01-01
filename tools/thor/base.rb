require "rbconfig"

module Mmdai

  module Build

    module Base
      include Thor::Actions

      INSTALL_ROOT_DIR = "install-root"

      def get_base_path()
        File.expand_path "#{File.dirname(__FILE__)}/../.."
      end

      def get_build_type()
        type = find_env_vars [ "VPVL2_BUILD_TYPE", "BUILD_TYPE" ]
        case type.downcase
          when "android" then :android
          when "debug" then :debug
          when "emscripten" then :emscripten
          when "flascc" then :flascc
          when "ios" then :ios
          when "nacl" then :nacl
          when "vs2012" then :vs2012
          when "vs2010" then :vs2010
          else :release
        end
      end

      def get_build_directory()
        "build-#{get_build_type.to_s}"
      end

      def get_build_path()
        File.expand_path "#{get_base_path}/#{get_directory_name}/#{get_build_directory}"
      end

    protected
      def invoke_build(extra_options = {})
        if options.key? "flag" then
          print_build_options extra_options
          puts
        else
          build_options = get_build_options get_build_type, extra_options
          empty_directory get_build_path
          start_build build_options, extra_options
        end
      end

      def invoke_clean(separated_arch = false)
        start_clean separated_arch
      end

      def make(argument = nil)
        if argument == nil then
          argument = "-j4"
        end
        run "make #{argument}"
      end

      def run_build(argument = nil)
        if get_build_type === :ios then
          run "xcodebuild"
        elsif is_ninja? then
          run "ninja #{argument}"
        elsif is_msvc? then
          run "msbuild /m:4 /t:Release"
        else
          make(argument)
        end
      end

      def is_ninja?
        return has_env_vars? [ "VPVL2_NINJA_BUILD", "NINJA_BUILD" ]
      end

      def is_msvc?
        type = get_build_type
        return type === :vs2012 || type === :vs2010 || ENV.key?("VCINSTALLDIR")
      end

      def is_darwin?
        return /^darwin/.match(RbConfig::CONFIG["target_os"]) && (get_build_type != :ios)
      end

      def is_executable?
        return false
      end

      def find_env_vars(candidates, default_value = "")
        candidates.each do |key| 
          if ENV.key? key then return ENV[key] end
        end
        return default_value
      end

      def has_env_vars?(candidates, default_value = false)
        candidates.each do |key| 
          if ENV.key? key then return true end
        end
        return default_value
      end

      def need_opengl_es?()
        case get_build_type
        when :android then
        when :emscripten then
        when :flascc then
        when :ios then
        when :nacl then
          return true
        end
        return is_msvc?
      end

    end # end of module Base

  end

end

