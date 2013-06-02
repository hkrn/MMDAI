module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_tag_name
    module Git
      include Thor::Actions

      def checkout
        if !options.key? "flag" then
          run "git clone #{get_uri} #{checkout_path}"
          inside checkout_path do
            run "git checkout #{get_tag_name}"
          end
        end
      end

      def checkout_path
        File.expand_path "#{File.dirname(__FILE__)}/../../#{get_directory_name}"
      end

    end

  end

end
