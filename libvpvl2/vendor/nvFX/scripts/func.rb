#!/usr/bin/env ruby1.9

#
# ag -w gl\\w+\s* | perl -ane 'my $l = $_; if ($l =~ /.+(gl\w+)\s*\(.+/) { print $1, "\n" }' | sort | uniq > func.txt
#

glew_header = IO.readlines("glew.h")
glge_header = IO.readlines("gl_gen.h")
found_names = []
not_found_names = []

IO.readlines("func.txt").each do |name|
  name.chomp!
  found = false
  glew_header.each do |l|
    result = /(.+PFN#{name.upcase}PROC.+)/x.match l
    if result then
      puts result[1]
      found_names.push name
      found = true
      break
    end
  end
  if not found then
    glge_header.each do |l|
# extern void (CODEGEN_FUNCPTR *_ptrc_glAccum)(GLenum, GLfloat);
      result = /extern\s+(.+)\s+\(CODEGEN_FUNCPTR\s+\*_ptrc_#{name}\)(.+)/x.match l
      if result then
        puts "typedef #{result[1]} (GLAPIENTRY * PFN#{name.upcase}PROC)#{result[2]}"
        found_names.push name
        found = true
        break
      end
    end
  end
  if not found and not not_found_names.index name then
    not_found_names.push name
  end
end

found_names.each do |name|
  puts "extern PFN#{name.upcase}PROC #{name};"
end
puts ""
puts "/*"
found_names.each do |name|
  puts "PFN#{name.upcase}PROC #{name} = 0;"
end
found_names.each do |name|
  puts "   #{name} = reinterpret_cast<PFN#{name.upcase}PROC>(resolver->resolve(\"#{name}\"));"
end
puts "*/"
puts ""
not_found_names.each do |name|
  puts "// #{name} is not found"
end

