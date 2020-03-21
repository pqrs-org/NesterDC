#!/usr/bin/env ruby


name_table = Hash.new

while l = $stdin.gets
  if /^(.+?) (<.+?>):/ =~ l then
    name_table[$1] = $2
  end
end

$stdin.seek(0, 0)

while l = $stdin.gets
  if /(.+?\s!\s)0x(.+)/ =~ l then
    name = name_table[$2.strip]
    if name.nil? then
      print l
    else
      print "#{$1}#{name} (#{$2})\n"
    end
  else
    print l
  end
end

