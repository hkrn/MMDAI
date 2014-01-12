require File.dirname(__FILE__) + '/base.rb'
require File.dirname(__FILE__) + '/git.rb'

module Mmdai

class Glm < Thor
  include Build::Base
  include VCS::Git

  desc "build", "build GLM (doesn't build actually)"
  def build
    checkout
  end

  desc "clean", "delete built GLM libraries (do nothing)"
  def clean
  end

protected
  def get_uri
    "https://github.com/g-truc/glm.git"
  end

  def get_directory_name
    "glm-src"
  end

  def get_tag_name
    "0.9.5.1"
  end

end

end

