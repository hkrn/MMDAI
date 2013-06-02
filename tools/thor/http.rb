require "net/ftp"
require "net/https"

module Mmdai

  module VCS

    # included class must implement below methods
    # * get_uri
    # * get_directory_name
    # * get_basename
    # * get_filename
    module Http
      include Thor::Actions

      def checkout
        if !options.key? "flag" then
          if not File.directory? checkout_path then
            path = checkout_path_with_dirname get_filename
            fetch_from_remote URI.parse(get_uri), path
            run "tar xzf #{path}"
            FileUtils.move checkout_path_with_dirname(get_basename), checkout_path
            remove_file path
          end
        end
      end

      def checkout_path
        checkout_path_with_dirname get_directory_name
      end

      def checkout_path_with_dirname(dirname)
        File.expand_path "#{File.dirname(__FILE__)}/../../#{dirname}"
      end

    private

      def fetch_from_remote(uri, path)
        if uri.scheme === "http" or uri.scheme === "https" then
          fetch_http uri, path
        elsif uri.scheme === "ftp" then
          fetch_ftp uri, path
        else
          raise ArgumentError, "Only http(s) or ftp is supported: #{uri}"
        end
      end

      def fetch_ftp(uri, path)
        Net::FTP.open uri.host do |ftp|
          ftp.login
          ftp.passive = true
          ftp.chdir File.dirname uri.path
          ftp.list File.basename(uri.path, ".tar.gz") + ".*"
          ftp.getbinaryfile File.basename(uri.path), path
        end
      end

      def fetch_http(uri, path, limit = 5)
        raise ArgumentError, 'HTTP redirect too deep' if limit == 0
        http = Net::HTTP.new uri.host
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        response = http.get uri.path
        case response
        when Net::HTTPRedirection
          fetch_http URI.parse(response['location']), path, limit - 1
        else
          File.open path, "wb" do |writer|
            writer.write response.body
          end
        end
      end

    end

  end

end
