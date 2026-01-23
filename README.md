# The open source all-screen keyboard
The first open source all-screen keyboard. This repository contains all the digital files used to produce the all-screen keyboard created by Inkbox Software.

## Material List
* Raspberry Pi 4 and up. (Program requires use of OpenGL 3 only supported by this generation of Pis onward)
* 14.1" wide screen touch display
* Pi Pico
* Battery (optional)
* Case and key files found in 3Dmodels folder
* Magnets
* Acrylic cores
* Various wires and cables to put everything together

### To enable SPI on Raspberry Pi running Ubuntu
Due to the 3840x1100 wide display resolution, Raspbian cannot be used. Ubutuntu then requires a little more settup to access all the Pi's features. 
In /boot/firmware/config.txt place the following text
```
enable_uart=1
dtoverlays=uart0-pi5
```
Restart, and serial is now accessible via /dev/ttyAMA0
Note this is different from Raspbian's serial file /dev/serial0

### To invert touch screeen output
Add the following rule as touchscreen.rules in /etc/udev/rules.d
```
ACTION=="add|change", KERNEL=="event*", ATTRS{name}=="ILITEK ILITEK-TP", ENV{LIBINPUT_CALIBRATION_MATRIX}="-1 0 1 0 -1 1 0 0 1"
```

### Installing required libraries
To install wiringpi go to the github repo (https://github.com/WiringPi/WiringPi) and under _Releases_ download the latest release wiringpi_X.XX_arm64.deb
Install with
```
sudo apt install wiringpi_x.xx_arm64.deb
```

The rest of the required libraries can be installed with
```
sudo apt install build-essential libgles2-mesa-dev libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-ttf-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavfilter-dev
```

### Starting the main keyboard program
Navigate to the directory
To build the program for the first time run
```
make build
```
Then run with
```
make run
```
or
```
sudo ./keyboard
```

The initial profile to load should be written in the _startup_ file.

### Designing Plugins
A plugin template has been created and stored in keyboard/src/plugins-dev/
Edit the plugin.cpp program to function as desired then compile your plugin via
```
make plugin name=myCustomPlugin src=plugin
```

### Keyboard Profile online designer
The latest version of this designer is hosted at https://notin.tokyo/ASK  
The code for this website is stored in keyboard/profileDesigner


