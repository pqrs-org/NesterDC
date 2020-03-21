#!/usr/bin/env ruby

sw = false

while l = gets
  sw = true if /^[a-z0-9]+ <_nes6502_execute>:$/ =~ l
  if sw && !(/:/ =~ l) then
    break
  end
  
  print l if sw 
end
