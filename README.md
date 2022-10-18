# picoclock
### for use with WAVESHARE RP2040-LCD-1.28
### RP2040 LCD 1.28
![uscn1](https://user-images.githubusercontent.com/26333559/196231673-cdbe89fb-14fd-46a9-b566-e3241b16d3c8.png)
![trde1](https://user-images.githubusercontent.com/26333559/196231689-c6d9e030-b088-4c9f-bef6-1a3cd4f5b1c6.png)

#### The button is simply plugged into GND and GP22 (h2)
#### With a button, attached the rp2040-lcd is controlled with the gyroscope

The images/fonts are in the ./img folder.
The file 'img2data.md' contains shell scripts for converting image data and fonts into header files.

#### Problems, bugs, tbd:

- analog seconds (pointer) not working properly
- battery display has to be adjusted depending on battery (still testing)

## To flash the image

`sudo picotool load ./build/main.uf2 -x --bus 1 --address XX`

Find the '--address' with:

`picotool info`


## Building the image

`cd picoclock;mkdir build;cd build;cmake ..;make`


