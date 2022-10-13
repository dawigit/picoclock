# picoclock
### for use with WAVESHARE RP2040-LCD-1.28
### RP2040 LCD 1.28
![pc_usa_cn](https://user-images.githubusercontent.com/26333559/195168002-3e70b9dc-ee9e-4af8-8cbc-15525633de07.jpg)



#### The button is simply plugged into GND and GP22 (h2)
(ky-040 test code is there)
E.g. the left image shows a simple button plugged into the h2 header.

#### With a button attached the rp2040-lcd is controlled with the gyroscopef

##### Every button press changes the mode:

- 1st: [WHITE]  None  [default mode, startup]
- 2nd: [CYAN]   Config (shows a cyan gyro-pointer, no use yet)
- 3rd: [YELLOW] ChangePosition (to change position in date/time/day of the week)
- 4rd: [ORANGE] EditPosition (to change value in that position: up/down)
- 5th: [RED/BLUE] ChangeTheme (to change the theme: left/right)


For now there are USA, China, Germany and Türkiye themes.
The images are in the ./examples folder.

The file 'img2data.md' contains a shell script (a function 'img2data')
so you can convert any image to to rgb565 format and then
into a header file (img.h) [in one turn].

#### Problems, bugs:

- analog seconds (pointer) not working properly
- battery display has to be adjusted depending on battery (still testing)
- theme colors selection for clock dial (seconds, every 5th second)
- gyro-control has to be improved/adjusted.
- gfx is slow

## To flash the image
sudo picotool load ./build/main.uf2 -x --bus 1 --address XX

You can find out the '--address' with:
picotool info
