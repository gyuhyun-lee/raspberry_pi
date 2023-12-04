# tty.usbserial-ABSCDJ3S should be given by typign ls /dev | grep usb
# seems like there is no setting for the parity bit?
minicom --8bit -b 115200 -w -D /dev/tty.usbserial-ABSCDJ3S


