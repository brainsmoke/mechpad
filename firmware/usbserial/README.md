# usb serial keypad

```
make -C ../libopencm3/ # if not done already
make
gdb-multiarch usbserial.elf
set mem inaccessible-by-default off
target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load
run
```
