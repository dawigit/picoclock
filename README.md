# picoclock
### WAVESHARE RP2040-LCD-1.28 / WAVESHARE RP2040-TOUCH-LCD-1.28

![s_flag](https://github.com/dawigit/picoclock/blob/main/img/s_flag.png)
![s_berry](https://github.com/dawigit/picoclock/blob/main/img/s_berry.png)
![s_ru](https://github.com/dawigit/picoclock/blob/main/img/s_ru.png)
![s_ua](https://github.com/dawigit/picoclock/blob/main/img/s_ua.png)

#### TOUCH
![s_qdtm](https://github.com/dawigit/picoclock/blob/main/img/s_qdtm.png) ![s_rotozcy](https://github.com/dawigit/picoclock/blob/main/img/s_rotozcy.png) ![s_rotozg](https://github.com/dawigit/picoclock/blob/main/img/s_rotozg.png) ![s_rotozr](https://github.com/dawigit/picoclock/blob/main/img/s_rotozr.png)
![s_bigfont](https://github.com/dawigit/picoclock/blob/main/img/s_bigfont.png) ![s_configicon_new](https://github.com/dawigit/picoclock/blob/main/img/s_configicon_new.png)
![s_spinmin](https://github.com/dawigit/picoclock/blob/main/img/s_spinmin.png) ![s_spindotw](https://github.com/dawigit/picoclock/blob/main/img/s_spindotw.png) ![s_spindotw_cn](https://github.com/dawigit/picoclock/blob/main/img/s_spindotw_cn.png)

#### NON-TOUCH
![s_gb](https://github.com/dawigit/picoclock/blob/main/img/s_gb.png) ![s_ch](https://github.com/dawigit/picoclock/blob/main/img/s_ch.png)  
![s_configicon](https://github.com/dawigit/picoclock/blob/main/img/s_configicon.png) ![s_configmenu](https://github.com/dawigit/picoclock/blob/main/img/s_configmenu.png)

https://github.com/dawigit/picoclock/tree/main/img/ancestral#readme old images

#### config (clock wise)
- ![conf_exit](https://github.com/dawigit/picoclock/blob/main/img/conf_exit.png)  exit config
- ![conf_background](https://github.com/dawigit/picoclock/blob/main/img/conf_background.png)	change background
- ![conf_rotozoom](https://github.com/dawigit/picoclock/blob/main/img/conf_rotozoom.png)  enable rotozoom
- ![conf_rotate](https://github.com/dawigit/picoclock/blob/main/img/conf_rotate.png)  rotate background [earth, eye]
- ![conf_save](https://github.com/dawigit/picoclock/blob/main/img/conf_save.png)  save config to flash
- ![conf_handstyle](https://github.com/dawigit/picoclock/blob/main/img/conf_handstyle.png)  change clock hand style [normal, alpha, textured]
- ![conf_clock](https://github.com/dawigit/picoclock/blob/main/img/conf_clock.png)  change clock hand texture
- ![conf_bender](https://github.com/dawigit/picoclock/blob/main/img/conf_bender.png)  dis-/enable second bender


![uscn1](https://user-images.githubusercontent.com/26333559/196231673-cdbe89fb-14fd-46a9-b566-e3241b16d3c8.png)
![trde1](https://user-images.githubusercontent.com/26333559/196231689-c6d9e030-b088-4c9f-bef6-1a3cd4f5b1c6.png)

#### Touch control (with RP2040-TOUCH-LCD-1.28)

#### The button is simply plugged into GND and GP22 (h2)
#### With a button, attached the rp2040-lcd is controlled with the gyroscope

The images/fonts are in the ./img folder.
The file 'img2data.md' contains shell scripts for converting image data and fonts into header files.

#### Problems, bugs, tbd:
- battery display has to be adjusted depending on battery type

## To flash the image

`sudo picotool load ./build/main.uf2 -x --bus 1 --address XX`

## To flash the image (touch)

`sudo picotool load ./uf2/rp2040-tlcd-128-watch.uf2 -x --bus 1 --address XX`


Find the '--address' with:

`picotool info`


## Building the image

`cd picoclock;mkdir build;cd build;cmake ..;make`

### commands:
- 'circle 1' to enable dynamic circles [0 to disable]
- and so on… the (boolean) values you can change between 0/1 are:
- 'sensors'		[show/hide sensor text]
- 'gyro'		  [show/hide gyrocross]
- 'bender'		[second pointer elastic]
- 'smooth'		[smoother/1frame delay for movement]
- 'insomnia'	[no sleep/ screen always on]
- 'circle'		[gyroscope changes circle, looks crappy atm]
- 'high'		[highpointer: pointer above text]
- 'alpha'		[alpha pointers]
- 'clock'		[shows/hides analog clock]

#### non boolean values
- 'theme'		[set theme (0-3)]
- 'light'		[set brightness (0-100)]
- 'hour'		[set hour (0-23]
- 'min'			[set minutes (0-59)]
- 'sec'			[set seconds (0-59)]
- 'dotw'		[set day of the week (0-6, 0=Sunday, 1=Monday, 6=Saturday)]
- 'year'		[set year (0-9999)]
- 'mon'			[set month (1-12)]
- 'day'			[set day (1-31)]
- 'spin'		[set degrees (-359,0,359) +/- when 'rota' is eanbled]
- 'deg'			[set the degree when (0-359) 'rota' is enabled]
- 'editpos'		[set editposition (0-8)]

### float values
- 'fspin'   [set fspin (+/-0.005) is a very slow rotation]

#### no args
- 'cir0'		[dynamic circle off]
- 'cir1'		[dynamic circle on]
- 'batmax'		[show bat max value (stdio)]
- 'batmin'		[show bat min value]
- 'save'		[save data]
- 'stat'		[show status (stdio)]
- 'roto'		[gfx_mode = rotozoom]
- 'rota'		[gfx_mode = rotating background for dynamic backgrounds]
- 'norm'		[gfx_mode = normal gfx, no rotation, no rotozoom]


### additional information
- all tools/scripts moved to folder 'tool'
- img2data.md -> tool/tools.md
- icons from openiconlibrary [https://sourceforge.net/projects/openiconlibrary]
- textures from opengameart.org (most of them)
- images from wikimedia.org
