# picoclock
### for use with WAVESHARE RP2040-LCD-1.28
### RP2040 LCD 1.28

![s_us](https://github.com/dawigit/picoclock/blob/main/img/s_us.png) ![s_us1](https://github.com/dawigit/picoclock/blob/main/img/s_us1.png) 
![s_cn](https://github.com/dawigit/picoclock/blob/main/img/s_cn.png) ![s_cn1](https://github.com/dawigit/picoclock/blob/main/img/s_cn1.png)
![s_de](https://github.com/dawigit/picoclock/blob/main/img/s_de.png) ![s_tr](https://github.com/dawigit/picoclock/blob/main/img/s_tr.png)


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

### News
- snapshots with 'snaps' script: `snaps 0 mysnapfilename` [/dev/ttyACM0]
- added shell: file 'pigsh' contains a simple bash script sending your input to /dev/ttyACMx
- 'pigsh 0' to connect to  /dev/ttyACM0 
- 'pigsh 1' to connect to  /dev/ttyACM1
- 'set DYNAMIC_CIRCLES 1' to enable dynamic circles [0 to disable]
- and so on… the (boolean) values you can change between 0/1 are:
- NO_SENSORS			[sensor text]
- NO_GYROCROSS			[show/hide gyrocross]
- SECOND_BENDER			[second pointer elasctic]
- SMOOTH_BACKGROUND		[smoother/1frame delay for movement]
- INSOMNIA				[never sleeps – for development only! stays on when bat full and not loading]
- DYNAMIC_CIRCLES		[gyroscope changes circle, looks crappy atm]
- all tools/scripts moved to folder 'tool'
- img2data.md -> tool/tools.md
- added bez2/3 curves
- added dynamic circles
- also added a few testing functions for bezier curves [they show all the lines, bigger dots]

 
