#!/usr/bin/env ruby1.9

#
# ag -w GL_[\\w_]+ | perl -ane 'my $l = $_; while ($l =~ s/(GL_[\w_]+)//) { print $1, "\n"; }' | sort | uniq > const.txt
#

glew = IO.read("glew.h")
glge = IO.read("gl_gen.h")
IO.readlines("const.txt").each do |name|
  name.chomp!
  result = /#define\s+#{name}\s+(0x)?(\w+)/.match glew
  if result then
    puts "static const GLenum #{name} = #{result[1]}#{result[2]};"
  else
    result = /#define\s*#{name}\s*(0x)?(\w+)/.match glge
    if result then
      puts "static const GLenum #{name} = #{result[1]}#{result[2]};"
    else
      puts "// #{name} is not found"
    end
  end
end

