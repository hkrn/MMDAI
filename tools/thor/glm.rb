require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

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
    "git://ogl-math.git.sourceforge.net/gitroot/ogl-math/ogl-math"
  end

  def get_directory_name
    "glm-src"
  end

  def get_tag_name
    "0.9.3.4"
  end

end

end
