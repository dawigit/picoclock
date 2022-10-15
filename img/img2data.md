

### usage: img2data IMAGEFILE SIZE*SIZE
#### example usage: img2data us16.png 16*16
#### generates file 'us16.h' with rgb565 data

#### you can add it to your .bashrc or whatever you use

function img2data()
{
oname=$(echo "$1" | cut -f 1 -d '.')
name=$(echo "$1" | cut -f 1 -d '.')rgb565
echo "name: $name"
convert $1 -rotate 180 -flop -alpha off -define bmp:subtype=RGB565 $name.bmp
#conversion: swap bytes (hi/lo)
dd conv=swab bs=138 skip=1 if=$name.bmp of=$oname.raw
touch $oname.h
echo "const unsigned char $oname[$2*2] = {" > $oname.h
hexdump -v -e '"0x" 1/1 "%02X" ","' $oname.raw >> $oname.h
rm $oname.raw
rm $name.bmp
echo "};" >> $oname.h
}


### usage: mkfontimagexy FONT X Y POINTSIZE
#### example usage: mkfontimagexy nf.ttc 31 40 27;
#### generates file 'nf.tcc.png' (2color)

#### POINTSIZE is responsible for X/Y of fonts so you have 'play' with the values a bit

A complete conversion process would be:
- install fonts
- cp /usr/share/fonts/opentype/noto/NotoSansCJK-Thin.ttc ./nf.ttc
- mkfontimagexy nf.ttc 31 40 27;
- eog nf.ttc.png

This generates 'nf.ttc.h' (aka Font40.h) and Font30.h is made alike.
Those fonts contain Latin, Cyrillic and a bunch of 'Chinese' characters – to write the weekdays.
A bit of show - generally it's NOT a good idea mixing fonts of different dimensions (rect vs. square)
or better: square-rect == waste-o-space

function mkfontimagexy()
{
convert -size $2x$((235*$3)) xc:black -font "$1" -pointsize $4 -fill white -draw "@b.txt" -threshold 0% -depth 1 -flip $1.png
convert $1.png -depth 1 $1.bmp
dd bs=150 skip=1 if=$1.bmp of=$1.raw
xxd -i $1.raw $1.h
rm $1.bmp
rm $1.raw
sed -i -r 's/\w+\[\]/Font'$3'_table\[\]/g' $1.h
sed -i -r 's/\w+_len/Font'$3'_len/g' $1.h
echo -e "\nsFONT Font"$3" = {\n  Font"$3"_table,\n  "$2", /* Width */\n  "$3", /* Height */\n};\n" >> $1.h
}


