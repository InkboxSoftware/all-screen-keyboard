# The open source all-screen keyboard
The first open source all-screen keyboard. This repository contains all the digital files used to produce the all-screen keyboard created by Inkbox Software in this video: https://www.youtube.com/watch?v=hqKPu2BEkI0

## Future of the all-screen keyboard?
The goal of this project is to create a robust DIY guide for creating an all-screen keyboard. Currentyle there are a couple of software issues that need to be resolved, but more importantly the keys and frames themselves need to be remolded in order to make them more suitable for viewing and mass production. Members of the community are looking towards how to create a cheaper version out of some other material than 3D printed resin. Once we solve these issues, version 2 will be released to the public via this repository. 

## Could I still make my own today?
Well, you probably don't want to. As of today (2025/7/11) the keys are much too tall, and so not all the keys are able to be viewed at once. But for reference here is the list of hardware I used:
* Orange Pi 5 Ultra (any Orange Pi 5 could be a drop in replacement, well, you may have to change which pins you use)
* PCB found in this project's PCB folder
* Top Shell and keys printed in transparent resin
* Bottom shell printed in regular PLA (available as a single piece or multiple parts for printing at home)

Currently start up requires three steps, starting the python program, setting the bluetooth advertisement appearance, and then running the main keyboard program. 

### To enable SPI on the Orange Pi 5:

open **/boot/extlinux/extlinux.conf**

Add the following text
```
FDTOVERLAYS /dtbs/rockchip/overlay/rk3588-spi0-m2-cs0-cs1-spidev.dtbo
/dtbs/rockchip/overlay/rk3588-spi1-m1-cs0-spidev.dtbo
/dtbs/rockchip/overlay/rk3588-spi4-m2-cs0-spidev.dtbo
```

### Running the bluetooth python program:
Ensure you have the proper dependencies installed
```
cd keyboard
make py
```

### Starting bluetooth:
Setting the appearance to 961 will have the Pi show up as a keyboard. 
```
bluetoothctl 
discoverable on
pairable on
advertise on
menu advertise
appearance 961
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


