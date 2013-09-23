#
# libxml2 is no longer used
#

require File.dirname(__FILE__) + '/configure.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai

class Libxml2 < Thor
  include Build::Configure
  include VCS::Http

  desc "build", "build libxml2"
  method_options :flag => :boolean
  def build
    checkout
    invoke_build
  end

  # use customized build rule
  desc "clean", "delete built libxml2 libraries"
  def clean
    inside get_build_path do
      make "clean"
      FileUtils.rmtree [ 'Makefile', INSTALL_ROOT_DIR ]
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
    "libxml2-src"
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
      "CC='clang -m32'"
    when :x86_64 then
      "CC='clang'"
    else
      ""
    end
  end

  def get_configure_path
    "../configure"
  end

  def get_debug_flag_for_configure
    ""
  end

  def get_directory_name
    "libxml2-src"
  end

  def run_msvc_build(build_options, build_type, build_directory, extra_options)
    path = "#{checkout_path}/win32"
    inside path do
      enable_debug = build_type === :debug ? "yes" : "no"
      run "nmake /f Makefile.msvc clean"
      run "cscript configure.js compiler=msvc prefix=..\\#{get_build_directory}\\install-root debug=#{enable_debug} ftp=no http=no html=no catalog=no docb=no iconv=no icu=no iso8859x=no zlib=no lzma=no"
      run "nmake /f Makefile.msvc"
      run "nmake /f Makefile.msvc install"
    end
  end

end

end

