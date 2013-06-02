
module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    module SVN
      include Thor::Actions

      def checkout
        if !options.key? "flag" then
          run "svn checkout #{get_uri} #{checkout_path}"
        end
      end

      def checkout_path
        File.expand_path "#{File.dirname(__FILE__)}/../../#{get_directory_name}"
      end

    end

  end

end
