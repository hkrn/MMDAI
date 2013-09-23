#
# GLEW is now bundled into libvpvl2 and this is no longer used.
#

require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/http.rb'

module Mmdai
class Glew < Thor
  include Build::Base
  include VCS::Http

  desc "build", "build GLEW"
  method_options :flag => :boolean
  def build
    checkout
    start_build
  end

  desc "clean", "delete built GLEW libraries"
  def clean
    if is_msvc? then
      inside "#{checkout_path}/build/vc10" do
        run "msbuild glew.sln /t:clean"
      end
    else
      inside checkout_path do
        make "clean"
      end
    end
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
    "glew-src"
  end

private
  def start_build(make_type = nil)
    if !options.key? "flag" then
      install_dir = "#{checkout_path}/#{get_build_directory}/#{INSTALL_ROOT_DIR}"
      rewrite_makefile checkout_path
      inside checkout_path do
        if is_msvc? then
          inside "#{checkout_path}/build/vc10" do
            run "msbuild glew.sln /t:build /p:configuration=#{get_build_type.to_s}"
          end
        else
          ENV["GLEW_DEST"] = install_dir
          make "uninstall"
          make "clean"
          make make_type
          make "install"
        end
      end
    end
  end

  def rewrite_makefile(base)
    if is_darwin?
      flags = "-arch i386 -arch x86_64"
      config_file_to_rewrite = "#{base}/config/Makefile.darwin"
      if get_build_type === :release then
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
      # disable stripping to create universal binary correctly
      ENV["STRIP"] = ""
    end
  end

  def delete_dynamic_libraries(install_dir)
    # darwin cannot link GLEW (universalized) statically on release
    if get_build_type === :release and not is_darwin? then
      [ "lib", "lib64" ].each do |dir|
        [ "so" ].each do |extension|
          FileUtils.rmtree [ Dir.glob("#{install_dir}/#{dir}/libGLEW*.#{extension}*") ]
        end
      end
    end
  end

end

end

