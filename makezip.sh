#!/bin/sh

stamp=$(date '+%Y-%m-%d')
baseurl="http://ayame.dyndns.org:20080/~tekezo/nesterdc/daily"
ndc="nesterdc-$stamp"

#----------------------------------------
# make elf
cd obj

cp nester.elf $ndc.elf
sh-elf-strip $ndc.elf
zip $ndc.elf.zip $ndc.elf

chmod 644 $ndc.elf.zip
mv $ndc.elf.zip ~/public_html/nesterdc/daily


#----------------------------------------
# make 1ST_READ.BIN
cd ..
./dcmake 1ST_READ.BIN
cd obj

zip $ndc-1ST_READ.BIN.zip 1ST_READ.BIN

chmod 644 $ndc-1ST_READ.BIN.zip
mv $ndc-1ST_READ.BIN.zip ~/public_html/nesterdc/daily

#----------------------------------------
cd ..
cat > index.html <<EOF 
<html>

<head>
 <title>daily NesterDC</title>
</head>
<body>
<h1>daily NesterDC</h1>

<h2>File</h2>
EOF

echo "<ul>" >> index.html
echo "<li><a href=\"$baseurl/$ndc.elf.zip\">$ndc.elf.zip</a></li> (for CD-R) " >> index.html
echo "<li><a href=\"$baseurl/$ndc-1ST_READ.BIN.zip\">$ndc-1ST_READ.BIN.zip</a></li> (for ip-slave, dcload) " >> index.html
echo "</ul>" >> index.html
echo "<ul>" >> index.html
echo "<li><a href=\"$baseurl/README\">README</a></li>" >> index.html
echo "<li><a href=\"$baseurl/README.keybind\">README.keybind</a></li>" >> index.html
echo "<li><a href=\"$baseurl/README.genie\">README.genie</a></li>" >> index.html
echo "</ul>" >> index.html

echo "<h2>Changes from last release</h2>" >> index.html

cat doc/changes.html >> index.html

cat >> index.html <<EOF
</body>
</html>
EOF


chmod 644 index.html
mv index.html ~/public_html/nesterdc/daily
#----------------------------------------
cp README ~/public_html/nesterdc/daily
chmod 644 ~/public_html/nesterdc/daily/README

cp README.keybind ~/public_html/nesterdc/daily
chmod 644 ~/public_html/nesterdc/daily/README.keybind

cp README.genie ~/public_html/nesterdc/daily
chmod 644 ~/public_html/nesterdc/daily/README.genie

