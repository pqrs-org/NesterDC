#!/usr/bin/env ruby

sw = false

while l = gets
  sw = true if /^[a-z0-9]+ <__ZN7NES_PPU..render_bg_regularEPt>:/ =~ l
  if sw && !(/:/ =~ l) then
    break
  end
  
  print l if sw 
end
