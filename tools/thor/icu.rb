require File.dirname(__FILE__) + '/configure.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Icu < Thor
  include Build::Configure
  include VCS::Http

  desc "build", "build ICU"
  method_options :flag => :boolean
  def build
    checkout
    extra_cflags = [
      "-DUCONFIG_NO_BREAK_ITERATION",
      "-DUCONFIG_NO_COLLATION",
      "-DUCONFIG_NO_FORMATTING",
      "-DUCONFIG_NO_TRANSLITERATION",
      "-DUCONFIG_NO_FILE_IO"
    ]
    invoke_build({ :extra_cflags => extra_cflags })
  end

  # use customized build rule
  desc "clean", "delete built ICU libraries"
  def clean
    if is_msvc? then
      inside "#{checkout_path}/source/allinone" do
        run "msbuild allinone.sln /t:clean"
      end
    else
      [ :debug, :release ].each do |build_type|
        build_directory = get_build_directory
        inside build_directory do
          make "clean"
          FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
        end
      end
    end
  end

protected
  def get_uri
    "http://download.icu-project.org/files/icu4c/51.2/#{get_filename}"
  end

  def get_basename
    "icu"
  end

  def get_filename
    "icu4c-51_2-src.tgz"
  end

  def get_build_options(build_type, extra_options)
    options = {
      :disable_icuio => nil,
      :disable_layout => nil,
      :disable_extras => nil,
      :disable_tests => nil,
      :disable_samples => nil,
      :with_data_packaging => "archive",
      :prefix => "#{get_build_directory}/#{INSTALL_ROOT_DIR}",
      :enable_release => nil,
      :enable_static => nil,
      :disable_shared => nil
    }
    return options
  end

  def get_configure_path
    return "../source/configure"
  end

  def get_debug_flag_for_configure
    return ""
  end

  def get_directory_name
    return "icu4c-src"
  end

private
  def run_msvc_build(build_options, build_type, build_directory, extra_options)
    inside "#{checkout_path}/source/allinone" do
      run "msbuild allinone.sln /t:build /p:configuration=#{build_type.to_s}"
    end
  end

end

end

