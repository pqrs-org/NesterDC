#!/usr/bin/env ruby

f = open(ARGV[0])

l = f.gets
unless /#ifndef/ =~ l then
  /(.+)\.h/ =~ ARGV[0]
  index = $1
  
  print "#ifndef __NES_MAPPER_#{index}_H\n"
  print "#define __NES_MAPPER_#{index}_H\n"
  print "\n"
  print "#include \"nes_mapper.h\"\n"
  
  print l
  while l = f.gets
    print l
  end
  
  print "#endif\n"
else
  print l
  while l = f.gets
    print l
  end
end
