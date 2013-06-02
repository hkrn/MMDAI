require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

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
    "git://github.com/g-truc/gli.git"
  end

  def get_directory_name
    "gli-src"
  end

  def get_tag_name
    "0.4.1.0"
  end

end

end
