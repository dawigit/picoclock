# picoclock
### for use with WAVESHARE RP2040-LCD-1.28
### RP2040 LCD 1.28
![pc_usa_cn](https://user-images.githubusercontent.com/26333559/195168002-3e70b9dc-ee9e-4af8-8cbc-15525633de07.jpg)



#### The button is simply plugged into GND and GP22 (h2)
(ky-040 test code is there)
E.g. the left image shows a simple button plugged into the h2 header.

#### With a button attached the rp2040-lcd is controlled with the gyroscope

For now there are USA, China, Germany and Türkiye themes.
The images/fonts are in the ./img folder.

The file 'img2data.md' contains shell scripts for converting image data and fonts into header files.

#### Problems, bugs:

- analog seconds (pointer) not working properly
- battery display has to be adjusted depending on battery (still testing)

## To flash the image

`sudo picotool load ./build/main.uf2 -x --bus 1 --address XX`

You can find out the '--address' with:

`picotool info`


## Building the image

`cd picoclock;mkdir build;cd build;cmake ..;make`


