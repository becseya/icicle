# Icicle

Your favourite christmas decoration

## Dependencies

`sudo apt-get install build-essential git gcc-avr avr-libc avrdude`

### If USBASP is giving you _"Operation not permitted"_

- `echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="1781", ATTRS{idProduct}=="0c9f", MODE="0666"' | sudo tee /etc/udev/rules.d/70-usbtiny.rules`
- Reboot or `restart udev`
- __Note:__ Vendor and product ID might differ in your case. Use `lsusb` to find out the value for your particular device.
