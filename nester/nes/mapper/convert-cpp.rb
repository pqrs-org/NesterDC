#!/usr/bin/env ruby

f = open(ARGV[0])

l = f.gets
unless /#include/ =~ l then
  /(.+)\.cpp/ =~ ARGV[0]
  index = $1
  
  print "#include \"types.h\"\n"
  print "#include \"nes.h\"\n"
  print "#include \"nes_mapper.h\"\n"
  print "#include \"#{index}.h\"\n"
end

print l
while l = f.gets
  print l
end
