# picoclock
![pc_usa_cn](https://user-images.githubusercontent.com/26333559/195168002-3e70b9dc-ee9e-4af8-8cbc-15525633de07.jpg)

uses RP2040-LCD-1.28

the button is simply plugged into GND and GP22 (h2)
(ky-040 test code is there)
e.g. the left image shows a simple button plugged into the h2 header.

with a button attached the rp2040-lcd is controlled with the gyroscope

1st button press:
- you enter 'editmode'
- color changes from WHITE to YELLOW.
- you can now change the position (up/down)
- in 'editmode' you can change the theme (left/right)

2nd button press:
- you change to 'changemode'
- color changes to ORANGE
- now with up/down (so x-axis) you change the values.

3rd button press:
- you change back to regular mode and the color is WHITE again.

for now there is us, china, german and turkye themes.
the images are in the ./examples folder.
img2data.md contains a shell script (a function 'img2data')
so you can convert any image to to rgb565 format and then
into a header file (img.h) [in one turn].

functions for sleep_on/sleep_off for the display 
were added in ./examples/LCD_1in28.c

all the other changes from the original waveshare code
are in ./examples/LCD_1in28_test.c

this is the file to start for changes!


problems, bugs:

- analog seconds (pointer) not working properly
- battery display has to be adjusted depending on battery (still testing)
- gyro-control has to be improved/adjusted.
- gfx is slow

to flash the image: 
(remember to exchange XX with your corresponding values)

sudo picotool load .uf2/wspicoclock.uf2 -x --bus 1 --address XX

you can find out the '--address' with:
picotool info

remember you have to hold 'boot' button, press 'reset', release the 'boot' button
before you can flash using picotool.


