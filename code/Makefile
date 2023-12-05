
MAKEFLAGS += --silent

AS_FLAGS = --warn --fatal-warnings
COMPILER_FLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 
BUILD_PATH = ../build
SDCARD_PATH = /Volumes/RPIBOOT

gcc : kernel8.img cleanup copy_to_sdcard 

kernel8.img :  
	aarch64-none-elf-as $(AS_FLAGS) entry.s -o $(BUILD_PATH)/entry.o

	aarch64-none-elf-gcc $(COMPILER_FLAGS) -c notmain.c -o $(BUILD_PATH)/notmain.o

	aarch64-none-elf-ld $(BUILD_PATH)/entry.o $(BUILD_PATH)/notmain.o -T memmap -o $(BUILD_PATH)/kernel8.elf
	# aarch64-none-elf-objcopy --srec-forceS3 $(BUILD_PATH)/kernel8.elf -O srec $(BUILD_PATH)/kernel8.srec
	aarch64-none-elf-objcopy $(BUILD_PATH)/kernel8.elf -O binary $(BUILD_PATH)/kernel8.img

	# output disassembly
	aarch64-none-elf-objdump -D $(BUILD_PATH)/kernel8.elf > kernel8.disasm

cleanup :
	rm -f $(BUILD_PATH)/*.o
	rm -f $(BUILD_PATH)/*.bin
	rm -f $(BUILD_PATH)/*.srec
	rm -f $(BUILD_PATH)/*.elf

# happens only if the sdcard(you can name however you want, just change the PRIBOOT part)
copy_to_sdcard : $(SDCARD_PATH)
	rm -f $(SDCARD_PATH)/kernel8.img 
	rm -f $(SDCARD_PATH)/config.txt 

	cp config.txt $(SDCARD_PATH)
	cp $(BUILD_PATH)/kernel8.img $(SDCARD_PATH)


