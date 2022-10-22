## pigsh

### usage: pigsh ttyACMn
#### example: pigsh 0
#### connect to your device, send commands, make screenshots


## snaps
### usage: snaps ACMNUMBER FILENAME
### example: snaps 0 mysnapshot
- starts a snapshot reading data from /dev/ttyACM0 [ACMNUMBER]
- the 'bmp' image will be converted (and flipped) to 'png'
- your output file name will be: 'mysnapshot.png' according to the above example
- if it hangs reset your device and repeat the process

snaps will call mksnap ACMNUMBER FILENAME
sleeps 2 seconds
then sends " " then "SNAPSHOT" to /dev/ttyACM[ACMNUMBER]
mksnap will open the 'png' with eog image viewer (if installed)

## snaps via ssh?
### example:
### ssh user@host 'cd picoclock/img; ../tool/snaps 1 mysnap'
### picoclock has to be in your home '/home/user/picoclock'
- connects to your host (like jim@raspi4 )
- changes to your 'picoclock/img' folder in your home (so images saved are there)
- calls 'snaps' script from 'tools' folder
- connection finished - ssh stuff done
##

### local image view: [you work from a linux system, ssh connected to a pi]
### ssh user@host 'cd picoclock/img; ../tool/snaps 1 mysnap'; eog mysnap.png
- assumptions:
- 'sshfs user@host:/home/user/picoclock' ./picoclock
- cd ~/picoclock/img
- my pi4 is headless, so no X installed.

### from your local terminal:
- ssh user@host 'cd picoclock/img; ../tool/snaps 1 mysnap'; eog mysnap.png



## obsolate [use snaps]
### snapshot

i left the old code if you want to play

### usage: snapshot CUTFILE
#### example: snapshot s_cn
#### converts data from device to image

### connect the RP2040-LCD-1.28 via USB to your Pi

#### in a terminal:

1. pigsh 0
2. press return once (the first input is ignored)
3. type 'SNAPSHOT' and press enter
4. in a 2nd terminal: 'cat /dev/ttyACM0 > snapshotcut'
    - the pointer should be 'frozen' now on the clock, data is transmitted
    - when the pointer starts moving again press 'control+c'
5. snapshot snapshotcut
6. hopefully creates snapshotcut.bmp
## obsolate


## img2data

### usage: img2data IMAGEFILE SIZE*SIZE
#### example: img2data us16.png 16*16
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

## mkfontimagexy

### usage: mkfontimagexy FONT X Y POINTSIZE
#### example: mkfontimagexy nf.ttc 31 40 27;
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
