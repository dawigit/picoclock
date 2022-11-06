# picoclock
### for use with WAVESHARE RP2040-LCD-1.28
### RP2040 LCD 1.28
![s_configicon](https://github.com/dawigit/picoclock/blob/main/img/s_configicon.png) ![s_configmenu](https://github.com/dawigit/picoclock/blob/main/img/s_configmenu.png)
![s_conf](https://github.com/dawigit/picoclock/blob/main/img/s_conf.png) ![s_conf1](https://github.com/dawigit/picoclock/blob/main/img/s_conf1.png)
![s_conf2](https://github.com/dawigit/picoclock/blob/main/img/s_conf2.png) ![s_conf3](https://github.com/dawigit/picoclock/blob/main/img/s_conf3.png)
![pwood](https://github.com/dawigit/picoclock/blob/main/img/pwood.png) ![pointerdemo](https://github.com/dawigit/picoclock/blob/main/img/pointerdemo.png)
![rotoz](https://github.com/dawigit/picoclock/blob/main/img/rotoz.png) ![rotoz1](https://github.com/dawigit/picoclock/blob/main/img/rotoz1.png)
![s_us](https://github.com/dawigit/picoclock/blob/main/img/s_us.png) ![s_us1](https://github.com/dawigit/picoclock/blob/main/img/s_us1.png)
![s_cn](https://github.com/dawigit/picoclock/blob/main/img/s_cn.png) ![s_cn1](https://github.com/dawigit/picoclock/blob/main/img/s_cn1.png)
![s_de](https://github.com/dawigit/picoclock/blob/main/img/s_de.png) ![s_tr](https://github.com/dawigit/picoclock/blob/main/img/s_tr.png)
![gyrocross](https://github.com/dawigit/picoclock/blob/main/img/gyrocross.png) ![gyrocrosscn](https://github.com/dawigit/picoclock/blob/main/img/gyrocrosscn.png)



![uscn1](https://user-images.githubusercontent.com/26333559/196231673-cdbe89fb-14fd-46a9-b566-e3241b16d3c8.png)
![trde1](https://user-images.githubusercontent.com/26333559/196231689-c6d9e030-b088-4c9f-bef6-1a3cd4f5b1c6.png)

#### The button is simply plugged into GND and GP22 (h2)
#### With a button, attached the rp2040-lcd is controlled with the gyroscope

The images/fonts are in the ./img folder.
The file 'img2data.md' contains shell scripts for converting image data and fonts into header files.

#### Problems, bugs, tbd:
- battery display has to be adjusted depending on battery (still testing)
- some crashes result in pico freezing at QMI8658_init() - remove battery

## To flash the image

`sudo picotool load ./build/main.uf2 -x --bus 1 --address XX`

Find the '--address' with:

`picotool info`


## Building the image

`cd picoclock;mkdir build;cd build;cmake ..;make`

### News
- load/save flash fixed. [restore save ram from flash now works]
- added new icons
- added config (next to center, opposite dotw)
- added more textures
- removed pointerdemo
- added textured pointers
- added alpha pointers
- added rotating background (themes 0,1)
- added rotozoom (from hagl [https://github.com/tuupola/hagl]) (cannon travel!)
- added "pico_bootsel_via_double_reset" in CMakeLists.txt
- more vars (values) now resist in 'noinit' ram
- battery display will flicker a lot at start (or when bat is lo) as it finds new min/max values
- when sleeping and a new lo-bat minimum was found, it saves (new minimum value)
- changed shell commands
- added flash ram
- added save: press the button for >3 sec when in center position (cursor/flag) to save save ram to flash ram
- gyrocross improved, shows ghosted old position
- battery display improved
- bending second pointer fixed at the star
- lcd_rect -> lcd_frame (and other changes in lcd)
- changed button system (internal)
- added battery types
- changed shell commands, see below
- snapshots with 'snaps' script: `snaps 0 mysnapfilename` [/dev/ttyACM0] (minicom must be stopped before)
- added shell: file 'pigsh' contains a simple bash script sending your input to /dev/ttyACMx (minicom has to be started before)
- 'pigsh 0' to connect to  /dev/ttyACM0
- 'pigsh 1' to connect to  /dev/ttyACM1
### commands:
- 'circle 1' to enable dynamic circles [0 to disable]
- and so on… the (boolean) values you can change between 0/1 are:
- 'sensors'		[show/hide sensor text]
- 'gyro'		  [show/hide gyrocross]
- 'bender'		[second pointer elastic]
- 'smooth'		[smoother/1frame delay for movement]
- 'insomnia'	[never sleeps – for development only! stays on when bat full and not loading]
- 'circle'		[gyroscope changes circle, looks crappy atm]
- 'deep'		[enables SLEEP_DEEP mode, sleeps deeper, can wake up every (1-30) second(s), sleeps deeper in time]
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
- 'cpst'    [set clock pointer style (0-2) normal,alpha,texture]

#### no args
- 'cir0'		[dynamic circle off]
- 'cir1'		[dynamic circle on]
- 'batmax'		[show bat max value (stdio)]
- 'batmin'		[show bat min value]
- 'save'		[save data]
- 'stat'		[show status (stdio)]
- 'roto'		[gfx_mode = rotozoom]
- 'rota'		[gfx_mode = rotating background for dynamic backgrounds (theme 0/1)]
- 'norm'		[gfx_mode = normal gfx, no rotation, no rotozoom]


### additional information
- all tools/scripts moved to folder 'tool'
- img2data.md -> tool/tools.md
- added bez2/3 curves
- added dynamic circles
- also added a few testing functions for bezier curves [they show all the lines, bigger dots]
- icons from openiconlibrary [https://sourceforge.net/projects/openiconlibrary]
- textures from opengameart.org (most of them)
- images from wikimedia.org
