require File.dirname(__FILE__) + '/configure.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Libav < Thor
  include Build::Configure
  include VCS::Git

  desc "debug", "build libav for debug"
  method_options :flag => :boolean
  def debug
    checkout
    invoke_build :debug, :separated_build => true
    make_universal_binaries :debug, false
  end

  desc "release", "build libav for release"
  method_options :flag => :boolean
  def release
    checkout
    invoke_build :release, :separated_build => true
    make_universal_binaries :release, false
  end

  desc "clean", "delete built libav libraries"
  def clean
    invoke_clean true
  end

protected
  def get_uri
    "git://git.libav.org/libav.git"
  end

  def get_directory_name
    "libav-src"
  end

  def get_tag_name
    "v9.1"
  end

  def get_arch_flag_for_configure(arch)
    case arch
    when :i386 then
      "--arch=i386 --cc='clang -m32'"
    when :x86_64 then
      "--arch=x86_64 --cc=clang"
    else
      ""
    end
  end

  def get_debug_flag_for_configure
    "--enable-debug=3 --disable-optimizations"
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

end

end
