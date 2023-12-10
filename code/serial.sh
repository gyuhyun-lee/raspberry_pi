# tty.usbserial-ABSCDJ3S should be given by typign ls /dev | grep usb
# -w wrapping
# --8bit = 8bit data(+1 start +1 stop, so 10 bits in total)

minicom --8bit -b 115200 -w -o -D /dev/tty.usbserial-ABSCDJ3S
# minicom --8bit -b 38400 -w -o -D /dev/tty.usbserial-ABSCDJ3S
# minicom --8bit -b 200 -w -D /dev/tty.usbserial-ABSCDJ3S


