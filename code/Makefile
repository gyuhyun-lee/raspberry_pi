MAKEFLAGS += --silent

ARMGNU ?= arm-none-eabi
#ARMGNU ?= arm-linux-gnueabi

AOPS = --warn --fatal-warnings
COPS = -Wall -Werror -O2 -nostdlib -nostartfiles -ffreestanding

all : clean kernel.img

clean :
	rm -f *.disasm
	rm -f *.img

kernel.img : notmain.s
	$(ARMGNU)-as $(AOPS) notmain.s -o kernel.img
	$(ARMGNU)-objdump -D kernel.img > kernel.disasm



