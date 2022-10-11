# usage: img2data IMAGEFILE SIZE*SIZE
# example usage: img2data us16.png 16*16
# generates file 'us16.h' with rgb565 data

# you can add it to your .bashrc or whatever you use

function img2data()
{
oname=$(echo "$1" | cut -f 1 -d '.')
name=$(echo "$1" | cut -f 1 -d '.')rgb565
echo "name: $name"
convert $1 -rotate 180 -flop -alpha off -define bmp:subtype=RGB565 $name.bmp
dd bs=138 skip=1 if=$name.bmp of=$oname.raw
touch $oname.h
echo "#pragma once
static char $oname[$2*2] = {" > $oname.h
hexdump -v -e '"0x" 1/1 "%02X" ","' $oname.raw >> $oname.h
rm $oname.raw
rm $name.bmp
echo "};" >> $oname.h
}
